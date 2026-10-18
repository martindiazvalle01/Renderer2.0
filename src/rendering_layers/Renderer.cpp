#include "Renderer.hpp"
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>

// Esta funcion demuesta el ciclo completo de la coordinacion en el renderizado de frames



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
    renderGraph.AddPass("DepthPrePass", {}, {"depth"},
    [this](vk::raii::CommandBuffer& cmd) {
        vk::RenderingAttachmentInfo colorAttachment{};
        colorAttachment.setImageView(renderGraph.GetResource("swapchain")->view);   // la view del frame actual
        colorAttachment.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
        colorAttachment.setLoadOp(vk::AttachmentLoadOp::eClear);
        colorAttachment.setStoreOp(vk::AttachmentStoreOp::eStore);
        colorAttachment.setClearValue(vk::ClearColorValue{std::array<float,4>{0.f, 0.f, 0.f, 1.f}});

        vk::RenderingInfo renderingInfo{};
        renderingInfo.setRenderArea({{0, 0}, swapchain.getExtent()});
        renderingInfo.setLayerCount(1);
        renderingInfo.setColorAttachments(colorAttachment);

        //cmd.beginRendering(renderingInfo);
        //cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineBiulder.buildPipeline(vk::Device &device));
        // set viewport/scissor, bind buffers y descriptors
        //cmd.draw(3, 1, 0, 0);
        //cmd.endRendering();
    });
}


