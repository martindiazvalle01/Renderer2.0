#pragma once
#include <PipelineBuilder.hpp>
#include <ShaderLoader.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "vulkan/vulkan.hpp"
#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"
#include "RenderDevice.hpp"
#include "RendererConfig.hpp"
#include "ShaderLoader.hpp"

class Renderer{
    public:
        void RenderFrame(RenderDevice& coreLayer,vk::Extent2D requestedExtent);

        Renderer(RenderDevice& coreLayer, const RendererConfig& config);

        void AddResources();

        void AddPasses();

        void CreatePipelines(const vk::raii::Device& device);

        vk::raii::ShaderModule createShaderModule(const std::string& filename, const vk::raii::Device& device);

    private:
        RenderGraph renderGraph;
        Swapchain swapchain;
        FrameManager frameManager;
        PipelineBuilder pipelineBuilder;
        ShaderLoader shaderLoader;
        // Variable de prueba, despues hay que cambiar y borrar
        vk::raii::Pipeline trianglePipeline{nullptr};
        
};