#include "Renderer.hpp"

#include "vulkan/vulkan.hpp"
#include <RenderGraph.hpp>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>





Renderer::Renderer(RenderDevice& coreLayer, const RendererConfig& config)
    : swapchain(coreLayer.getSurface().get(),
                coreLayer.getPhysicalDevice(),
                coreLayer.getLogicalDevice().get(),
                config.extent,
                coreLayer.getLogicalDevice().getGraphicsIndex(),
                coreLayer.getLogicalDevice().getPresentIndex()),
      frameManager(coreLayer.getLogicalDevice().get(),
                   coreLayer.getLogicalDevice().getGraphicsIndex())
{
    // Un renderFinished por imagen del swapchain
    frameManager.ensureRenderFinishedSemaphores(coreLayer.context().device, swapchain.getImages().size());
    AddResources();
    CreatePipelines(coreLayer.context().device);
    AddPasses();
    
}


void Renderer::AddResources(){
    auto extent = swapchain.getExtent();

    renderGraph.AddResource("depth", vk::Format::eD32Sfloat, extent,
                  vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
                  vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined);

    


    renderGraph.AddResource("swapchain", swapchain.getFormat(), extent,
                  vk::ImageUsageFlagBits::eColorAttachment,
                  vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);


}

void Renderer::AddPasses(){
    //Por ahora este Pass es solo de prueba, no es el diseño final
    renderGraph.AddPass("TrianglePass", {}, {"swapchain"},
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

        cmd.beginRendering(info);
        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);
        cmd.setViewport(0, vk::Viewport{0.f, 0.f, (float)extent.width, (float)extent.height, 0.f, 1.f});
        cmd.setScissor(0, vk::Rect2D{{0, 0}, extent});
        cmd.draw(3, 1, 0, 0);
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

    vk::raii::PipelineLayout trianglePipelineLayout(device, layoutInfo);

    pipelineBuilder.setPipelineLayout(trianglePipelineLayout);

    // Mismo módulo para las dos etapas, cada una con su entry point
    pipelineBuilder.setShaders(*triangleShader, *triangleShader, "vertexMain", "fragmentMain");

    pipelineBuilder.setPolygonMode(vk::PolygonMode::eFill);

    pipelineBuilder.setInputTopology(vk::PrimitiveTopology::eTriangleList);

    pipelineBuilder.setCullMode(vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise);

    pipelineBuilder.setMultisamplingNone();

    pipelineBuilder.disableBending();

    pipelineBuilder.disableDepthTest();

    pipelineBuilder.setColorAttachmentFormat(swapchain.getFormat());

    trianglePipeline = pipelineBuilder.buildPipeline(device);
}


void Renderer::RenderFrame(RenderDevice& coreLayer,vk::Extent2D requestedExtent){
    
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
    renderGraph.Compile(coreLayer.context().device, coreLayer.context().allocator);

    renderGraph.setImportedImage("swapchain", swapchain.getImages()[index], *swapchain.getImageViews()[index]);

    renderGraph.Execute(frameManager.currentCmd());

    bool result = frameManager.submitFrame(coreLayer.context().graphicsQueue, swapchain.get(), index);

    if(!result){
        swapchain.recreate(coreLayer.context().device, coreLayer.context().physicalDevice, coreLayer.context().surface, 
                           coreLayer.context().graphicsIndex, coreLayer.context().presentIndex, requestedExtent);
        frameManager.ensureRenderFinishedSemaphores(coreLayer.context().device, swapchain.getImages().size());
    }
    
}