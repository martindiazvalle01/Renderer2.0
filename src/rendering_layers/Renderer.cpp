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
    AddResources();
    CreatePipelines(coreLayer.getLogicalDevice().get());
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
        color.setClearValue(vk::ClearColorValue{std::array<float, 4>{0.f, 0.f, 0.f, 1.f}});

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
    // un Shader de prueba 
    vk::ShaderModule triangleFragShader = shaderLoader.createShaderModule("/shaders/colored_triangle.frag.spv", device);

    vk::ShaderModule triangleVertShader = shaderLoader.createShaderModule("/shaders/colored_triangle.vert.spv", device);

    vk::PipelineLayoutCreateInfo layoutInfo{};

    vk::raii::PipelineLayout trianglePipelineLayout(device,layoutInfo);

    pipelineBuilder.setPipelineLayout(trianglePipelineLayout);

    pipelineBuilder.setShaders(triangleVertShader,triangleFragShader);

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
        return;
    }
    
    uint32_t index = *imageIndex;

    renderGraph.setImportedImage("swapchain", swapchain.getImages()[index], *swapchain.getImageViews()[index]);

    renderGraph.Execute(frameManager.currentCmd());

    bool result = frameManager.submitFrame(coreLayer.context().graphicsQueue, swapchain.get(), index);

    if(!result){
        swapchain.recreate(coreLayer.context().device, coreLayer.context().physicalDevice, coreLayer.context().surface, 
                           coreLayer.context().graphicsIndex, coreLayer.context().presentIndex, requestedExtent);
    }
    
}