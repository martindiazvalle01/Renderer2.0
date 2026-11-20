#include "Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include <Camera.hpp>
#include <MeshBufferManager.hpp>
#include <RenderGraph.hpp>
#include <cstdint>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_raii.hpp>





void Renderer::init(RenderDevice& coreLayer, const RendererConfig& config){

    swapchain.init(coreLayer.getSurface().get(),
                coreLayer.getPhysicalDevice(),
                coreLayer.getLogicalDevice().get(),
                config.extent,
                coreLayer.getLogicalDevice().getGraphicsIndex(),
                coreLayer.getLogicalDevice().getPresentIndex());


    frameManager.init(coreLayer.getLogicalDevice().get(),
                   coreLayer.getLogicalDevice().getGraphicsIndex());

    meshBufferManager.init(coreLayer.context().allocator,coreLayer.context().device,coreLayer.context().graphicsQueue,coreLayer.context().graphicsIndex);
    // Un renderFinished por imagen del swapchain
    frameManager.ensureRenderFinishedSemaphores(coreLayer.context().device, swapchain.getImages().size());
    
    AddResources();
    CreatePipelines(coreLayer.context().device);
    AddPasses();
    renderGraph.Compile(coreLayer.context().device, coreLayer.context().allocator);
    sceneDataManager.init(meshBufferManager, coreLayer.context().device);
    
}


void Renderer::AddResources(){
    auto extent = swapchain.getExtent();

    renderGraph.AddResource("depth", vk::Format::eD32Sfloat, extent,
                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
                  vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined);

    


    renderGraph.AddResource("swapchain", swapchain.getFormat(), extent,
                  vk::ImageUsageFlagBits::eColorAttachment,
                  vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,true);


}

void Renderer::AddPasses(){
    //Por ahora este Pass es solo de prueba, no es el diseño final
    renderGraph.AddPass("TrianglePass", {}, {"swapchain","depth"},
    [this](vk::raii::CommandBuffer& cmd) {
        vk::ImageView view = renderGraph.GetResource("swapchain")->view;
        auto extent = swapchain.getExtent();

        vk::RenderingAttachmentInfo color{};
        color.setImageView(view);
        color.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
        color.setLoadOp(vk::AttachmentLoadOp::eClear);
        color.setStoreOp(vk::AttachmentStoreOp::eStore);
        color.setClearValue(vk::ClearColorValue{std::array<float, 4>{0.2f, 0.0f, 0.3f, 1.f}}); // morado

        vk::RenderingInfo info{};
        info.setRenderArea({{0, 0}, extent});
        info.setLayerCount(1);
        info.setColorAttachments(color);

        vk::RenderingAttachmentInfo depthInfo;
        depthInfo.imageView = renderGraph.GetResource("depth")->view;
        depthInfo.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;  
        depthInfo.setLoadOp(vk::AttachmentLoadOp::eClear);
        depthInfo.setStoreOp(vk::AttachmentStoreOp::eDontCare);
        depthInfo.clearValue = vk::ClearDepthStencilValue{1.0f,0};

        info.setPDepthAttachment( &depthInfo);

        cmd.beginRendering(info);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);
        cmd.setViewport(0, vk::Viewport{0.f, 0.f, (float)extent.width, (float)extent.height, 0.f, 1.f});
        cmd.setScissor(0, vk::Rect2D{{0, 0}, extent});

        PushConstantsData pc{};
        pc.modelMatrix   = glm::mat4(1.f);
        pc.vertexAddress = meshBuffers.vertexBufferAddress;
        pc.sceneAddress  = sceneDataManager.getAdress(currentIndex);
        cmd.pushConstants<PushConstantsData>(*layout, vk::ShaderStageFlagBits::eVertex, 0, pc);


        cmd.bindIndexBuffer(meshBuffers.indexBuffer.buffer, 0, vk::IndexType::eUint32);
        cmd.drawIndexed(meshBuffers.indexCount, 1, 0, 0, 0);
        cmd.endRendering();
    });

    
}


void Renderer::CreatePipelines(const vk::raii::Device& device){
    // Un solo .spv con las dos etapas (vertexMain y fragmentMain).
    // Se guarda como vk::raii::ShaderModule para que siga vivo hasta buildPipeline;
    // se destruye solo al salir de la función, cuando el pipeline ya está creado.
    vk::raii::ShaderModule triangleShader =
        ShaderLoader::createShaderModule("assets/shaders/core/colored_triangle.spv", device);

    vk::PipelineLayoutCreateInfo layoutInfo{};

    //Esta es simplemente la info del tamaño de constantData y que stage lo va a usar, por ahora solo vertex
    vk::PushConstantRange range{vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstantsData)};
    layoutInfo.setPushConstantRanges(range);


    vk::raii::PipelineLayout trianglePipelineLayout(device, layoutInfo);
    

    pipelineBuilder.setPipelineLayout(trianglePipelineLayout);

    // Mismo módulo para las dos etapas, cada una con su entry point
    pipelineBuilder.setShaders(*triangleShader, *triangleShader, "vertexMain", "fragmentMain");

    pipelineBuilder.setPolygonMode(vk::PolygonMode::eFill);

    pipelineBuilder.setInputTopology(vk::PrimitiveTopology::eTriangleList);

    pipelineBuilder.setCullMode(vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise);

    pipelineBuilder.setMultisamplingNone();

    pipelineBuilder.disableBending();

    pipelineBuilder.setDepthFormat(vk::Format::eD32Sfloat);
    vk::PipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.setDepthTestEnable(vk::True);
    depthStencil.setDepthWriteEnable(vk::True);
    depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
    depthStencil.setDepthBoundsTestEnable(vk::False);
    depthStencil.setStencilTestEnable(vk::False);

    pipelineBuilder.setDepthStencil(depthStencil);

    pipelineBuilder.setColorAttachmentFormat(swapchain.getFormat());

    trianglePipeline = pipelineBuilder.buildPipeline(device);
    layout.swap(trianglePipelineLayout);
}

void Renderer::PushConstants(vk::raii::CommandBuffer& cmd, uint32_t frameIndex){
    PushConstantsData pc{};
    pc.modelMatrix = glm::mat4(1.f);
    pc.vertexAddress = meshBuffers.vertexBufferAddress;
    pc.sceneAddress = sceneDataManager.getAdress(frameIndex);
 
    cmd.pushConstants<PushConstantsData>(*layout,
                                     vk::ShaderStageFlagBits::eVertex,
                                     0, pc);
    pc.sceneAddress = 0;
    pc.vertexAddress = 0;
}

void Renderer::RenderFrame(RenderDevice& coreLayer,vk::Extent2D requestedExtent, Camera camera){
    
    // Empezamos el frame tomando el imageIndex
    auto imageIndex = frameManager.beginFrame(coreLayer.context().device, swapchain.get());

    //Revisamos si retorna algo
    if(!imageIndex){
        swapchain.recreate(coreLayer.context().device, coreLayer.context().physicalDevice, coreLayer.context().surface, 
                           coreLayer.context().graphicsIndex, coreLayer.context().presentIndex, requestedExtent);
        frameManager.ensureRenderFinishedSemaphores(coreLayer.context().device, swapchain.getImages().size());
        return;
    }

    uint32_t index = *imageIndex;

    currentIndex = index;


  

    float aspect = (float)requestedExtent.width / (float)requestedExtent.height;
    glm::mat4 projectionMatrix = camera.getProjectionMatrix(aspect);
    projectionMatrix[1][1] *= -1;
    glm::mat4 viewProj = projectionMatrix * camera.getViewMatrix();
  
    updatePushConstants(viewProj, glm::vec4(camera.getPosition(), 1.f));
    

 
    

    renderGraph.setImportedImage("swapchain", swapchain.getImages()[index], *swapchain.getImageViews()[index]);

    renderGraph.Execute(frameManager.currentCmd());

    bool result = frameManager.submitFrame(coreLayer.context().graphicsQueue, swapchain.get(), index);

    if(!result){
        swapchain.recreate(coreLayer.context().device, coreLayer.context().physicalDevice, coreLayer.context().surface, 
                           coreLayer.context().graphicsIndex, coreLayer.context().presentIndex, requestedExtent);
        frameManager.ensureRenderFinishedSemaphores(coreLayer.context().device, swapchain.getImages().size());
    }
    
    
}

//Esta clase literalmente sube el mesh a la GPU y borra el 
void Renderer::uploadMeshToGpu(std::span<uint32_t> indices, std::span<Vertex> vertices){
    meshBuffers = meshBufferManager.uploadMesh(indices,vertices);
    
    
}

void Renderer::destroyMesh(){
    meshBufferManager.destroyMesh(meshBuffers);
}

