#include "FrameManager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>


void FrameManager::createContext(const vk::raii::Device& device, uint32_t queueFamilyIndex){
    for(int i = 0;i < MaxFramesInFlight; i++){
        frames[i].pool = createCommandPool(device,queueFamilyIndex);
        // Los ultimos 2 parametros, el count y el level estan por default
        // Si en el futuro se agrega multi threading hay que cambiar
        frames[i].cmd = createCommandBuffer(device, frames[i].pool);
        createFrameSemaphores(device,frames[i]);
    }
    createTimelineSemaphore(device);
}

vk::raii::CommandPool FrameManager::createCommandPool(const vk::raii::Device& device, uint32_t queueFamilyIndex) {
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    return vk::raii::CommandPool(device, poolInfo);
}

std::vector<vk::raii::CommandBuffer> FrameManager::createCommandBuffer(const vk::raii::Device& device,const vk::CommandPool& pool,uint32_t count,
                                                             vk::CommandBufferLevel level){
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.commandPool        = pool;
    allocInfo.level              = level;
    allocInfo.commandBufferCount = count;

    vk::raii::CommandBuffers commandBuffers(device, allocInfo);
    std::vector<vk::raii::CommandBuffer> listaBuffers;
    listaBuffers.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        listaBuffers.push_back(std::move(commandBuffers[i]));
    }

    return listaBuffers;

}
//Semaforos Binarios (0,1) que son para indicar que la imagen esta lista para usarse o si se termino de usar
void FrameManager::createFrameSemaphores(const vk::raii::Device& device, FrameContext& frame) {
    frame.imageAvailable = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo{});
    frame.renderFinished = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo{});
}
// Semaforo por tiempo el cual se usa de forma global para indicar que cosa termino en que tiempo
void FrameManager::createTimelineSemaphore(const vk::raii::Device& device) {
    vk::StructureChain<vk::SemaphoreCreateInfo, vk::SemaphoreTypeCreateInfo> timelineChain{
        {},
        { vk::SemaphoreType::eTimeline, 0 }
    };
    timeline = vk::raii::Semaphore(device, timelineChain.get<vk::SemaphoreCreateInfo>());
}

//Esta funcion Realiza todo lo que debe ocurrir antes de grabar comandos de dibujo
std::optional<uint32_t> FrameManager::beginFrame(const vk::raii::Device& device,vk::raii::SwapchainKHR& swapchain){
    uint64_t currentFrameIndex = frameCount % MaxFramesInFlight;
    FrameContext& fc = frames[currentFrameIndex];

    //revisamos cuanto tiempo hay que esperar para iniciar, si es 0 todo esto se salta
    if (fc.retireValue != 0) {
        vk::SemaphoreWaitInfo waitInfo{};
        waitInfo.semaphoreCount = 1;
        waitInfo.pSemaphores    = &*timeline;  // puntero al handle crudo (osea el VKFRAMEBUFFER)
        waitInfo.pValues        = &fc.retireValue;

        vk::Result result = device.waitSemaphores(waitInfo, UINT64_MAX);
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("Timeout esperando el timeline semaphore");
        }
    }

    
     uint32_t imageIndex;
     // Todo esto se puede mejorar haciendo que suboptimal se trate de forma diferente a outofdate
     // Pero mejor dejarlo para despues, es algo simple que hace que no se pegue la pantalla por un frame cuando se cambia la resolucion
     // Eso es tarea para Martin del futuro
    try {
        //Se coloca un nullPtr en el ultimo porque no hay fence y se usa UINT64_MAX porque no hay timeout
        auto [result, idx] = swapchain.acquireNextImage(UINT64_MAX, *fc.imageAvailable, nullptr);
        if (result == vk::Result::eSuboptimalKHR) {
            return std::nullopt;  // pedimos que se recree la swapchain
        }
        imageIndex = idx;
    } catch (const vk::OutOfDateKHRError&) {
        return std::nullopt; // Atrapamos el error en lugar de dejar que pare la aplicacion
    }

    //Esto simplemente activa el modo grabacion del framebuffer para empezar a grabar comandos
    vk::CommandBufferBeginInfo beginInfo{};
    fc.cmd[0].begin(beginInfo);

    return imageIndex;

}


bool FrameManager::submitFrame( const vk::raii::Queue& graphicsQueue,
                                vk::raii::SwapchainKHR& swapchain,
                                uint32_t imageIndex) {
    uint64_t currentFrameIndex = frameCount % MaxFramesInFlight;
    FrameContext& fc = frames[currentFrameIndex];

    // Terminamos la grabación del command buffer
    fc.cmd[0].end();

    // Armamos la info de wait que esperar a que la imagen esté disponible
    vk::SemaphoreSubmitInfo waitInfo{};
    waitInfo.semaphore = *fc.imageAvailable;
    waitInfo.stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    // Le asignamos al nextValue el valor que el timeline va a tener cuando este frame termine
    uint64_t frameComplete = nextSubmitValue++;

    // Armamos los 2 signals: timeline (para throttling CPU) + renderFinished (para present)
    std::array<vk::SemaphoreSubmitInfo, 2> signalInfos{};
    signalInfos[0].semaphore = *timeline;
    signalInfos[0].value     = frameComplete;   // solo relevante para timeline semaphores
    signalInfos[0].stageMask = vk::PipelineStageFlagBits2::eAllCommands;

    signalInfos[1].semaphore = *fc.renderFinished;
    signalInfos[1].stageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput;

    // Info del command buffer a subir
    vk::CommandBufferSubmitInfo cmdInfo{};
    cmdInfo.commandBuffer = *fc.cmd[0];

    // Aqui se arma el submit completo
    vk::SubmitInfo2 submit{};
    submit.waitSemaphoreInfoCount   = 1;
    submit.pWaitSemaphoreInfos      = &waitInfo;
    submit.commandBufferInfoCount   = 1;
    submit.pCommandBufferInfos      = &cmdInfo;
    submit.signalSemaphoreInfoCount = static_cast<uint32_t>(signalInfos.size());
    submit.pSignalSemaphoreInfos    = signalInfos.data();

    graphicsQueue.submit2(submit);

    // Guardamos el retireValue para la próxima vez que se reuse este slot
    fc.retireValue = frameComplete;

    // Armamos el presentInfo (osea la informacion actual del frame en tiempo presente)
    vk::PresentInfoKHR presentInfo{};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = &*fc.renderFinished;
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &*swapchain;
    presentInfo.pImageIndices      = &imageIndex;

  
  
    vk::Result presentResult;
    try {
        presentResult = graphicsQueue.presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError&) {
        frameCount++;  // el fram avanzó, solo el present falló
        return false;
    }

    if (presentResult == vk::Result::eSuboptimalKHR) {
        frameCount++;
        return false;  
    }


    // Avanzamos el contador de frame
    frameCount++;
    
    return true;
}

// El constructor de clase, no se si en algun momento necesite el create context para algo aparte del constructor
// pero por si acaso se queda como funcion individual
FrameManager::FrameManager(const vk::raii::Device& device, uint32_t queueFamilyIndex) {
    createContext(device, queueFamilyIndex);
}