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
// Semaforo binario por frame: imageAvailable lo señaliza acquire y lo espera nuestro submit,
// asi que el timeline del frame lo cubre y puede ir por frame
void FrameManager::createFrameSemaphores(const vk::raii::Device& device, FrameContext& frame) {
    frame.imageAvailable = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo{});
    // CAMBIO: renderFinished ya no se crea aqui
}


void FrameManager::ensureRenderFinishedSemaphores(const vk::raii::Device& device, size_t imageCount) {
    // Solo se agregan, nunca se destruyen: alguno podria seguir esperandolo
    // un present del swapchain anterior. Si el swapchain nuevo tiene menos
    // imagenes, los que sobran simplemente quedan sin usar.
    while (renderFinished.size() < imageCount) {
        renderFinished.emplace_back(device, vk::SemaphoreCreateInfo{});
    }
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
std::optional<uint32_t> FrameManager::beginFrame(const vk::raii::Device& device,const vk::raii::SwapchainKHR& swapchain){
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
    try {
        //Se coloca un nullPtr en el ultimo porque no hay fence y se usa UINT64_MAX porque no hay timeout
        [[maybe_unused]] auto [result, idx] = swapchain.acquireNextImage(UINT64_MAX, *fc.imageAvailable, nullptr);
        // si acquire devuelve eSuboptimalKHR, igual nos entrega una imagen y va a
        // señalizar imageAvailable. Antes se abandonaba el frame: el semaforo quedaba
        // señalizado sin que nadie lo esperara y la imagen quedaba tomada para siempre.
        // Ahora se dibuja normal: present tambien va a devolver suboptimal y ahi se recrea.
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
                                const vk::raii::SwapchainKHR& swapchain,
                                uint32_t imageIndex) {
                                    
    uint64_t currentFrameIndex = frameCount % MaxFramesInFlight;
    FrameContext& fc = frames[currentFrameIndex];

    // Terminamos la grabación del command buffer
    fc.cmd[0].end();

    // Armamos la info de wait que esperar a que la imagen esté disponible
    vk::SemaphoreSubmitInfo waitInfo{};
    waitInfo.semaphore = *fc.imageAvailable;
    waitInfo.stageMask = vk::PipelineStageFlagBits2::eAllCommands;

    // Le asignamos al nextValue el valor que el timeline va a tener cuando este frame termine
    uint64_t frameComplete = nextSubmitValue++;

    // Armamos los 2 signals: timeline (para throttling CPU) + renderFinished (para present)
    std::array<vk::SemaphoreSubmitInfo, 2> signalInfos{};
    signalInfos[0].semaphore = *timeline;
    signalInfos[0].value     = frameComplete;   // solo relevante para timeline semaphores
    signalInfos[0].stageMask = vk::PipelineStageFlagBits2::eAllCommands;

    signalInfos[1].semaphore = *renderFinished[imageIndex];   // CAMBIO: por imagen, no por frame
    signalInfos[1].stageMask = vk::PipelineStageFlagBits2::eAllCommands;

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
    presentInfo.pWaitSemaphores    = &*renderFinished[imageIndex];   // CAMBIO: el mismo que señalizo el submit
    presentInfo.swapchainCount     = 1;
    presentInfo.pSwapchains        = &*swapchain;
    presentInfo.pImageIndices      = &imageIndex;

    vk::Result presentResult;
    try {
        presentResult = graphicsQueue.presentKHR(presentInfo);
    } catch (const vk::OutOfDateKHRError&) {
        frameCount++;  // el frame avanzó, solo el present falló
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

vk::raii::CommandBuffer& FrameManager::currentCmd() {
    return frames[frameCount % MaxFramesInFlight].cmd[0];
}