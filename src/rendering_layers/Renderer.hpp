#pragma once
#include <PipelineBuilder.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "vulkan/vulkan.hpp"
#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"
#include "RenderDevice.hpp"
#include "RendererConfig.hpp"

class Renderer{
    public:
        void RenderFrame(vk::raii::Device& device, vk::Queue graphicsQueue,vk::Queue presentQueue);

        Renderer(RenderDevice& coreLayer, const RendererConfig& config);

        void AddResources();

        void AddPasses();

        void CreatePipeline();

    private:
        RenderGraph renderGraph;
        Swapchain swapchain;
        FrameManager frameManager;
        PipelineBuilder pipelineBiulder;
        

};