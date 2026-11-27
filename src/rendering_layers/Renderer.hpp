#pragma once
#include <AssetTextureManager.hpp>
#include <SceneDataManager.hpp>
#include <PipelineBuilder.hpp>
#include <ShaderLoader.hpp>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>
#include "vulkan/vulkan.hpp"
#include "RenderGraph.hpp"
#include "Swapchain.hpp"
#include "FrameManager.hpp"
#include "RenderDevice.hpp"
#include "RendererConfig.hpp"
#include "ShaderLoader.hpp"
#include "MeshBufferManager.hpp"
#include "Camera.hpp"


class Renderer{
    public:
        void RenderFrame(RenderDevice& coreLayer,vk::Extent2D requestedExtent, Camera camera);

        void init(RenderDevice& coreLayer, const RendererConfig& config);

        void AddResources();

        void AddPasses();

        void CreatePipelines(const vk::raii::Device& device);

        vk::raii::ShaderModule createShaderModule(const std::string& filename, const vk::raii::Device& device);

        void uploadMeshToGpu(std::span<uint32_t> indices, std::span<Vertex> vertices);
        void destroyMesh();
        struct PushConstantsData{
            glm::mat4 modelMatrix;
            vk::DeviceAddress vertexAddress;
            vk::DeviceAddress sceneAddress;
            uint32_t baseColorTex;
            uint32_t normalTex;
            uint32_t metallicRoughnessTex;
            uint32_t emissiveTex;
            uint32_t occlusionTex;
        };

        void PushConstants(vk::raii::CommandBuffer& cmd,uint32_t frameIndex);
    ~Renderer(){
        renderGraph = RenderGraph{};
        
    }

    void updatePushConstants(glm::mat4 viewProj,glm::vec4 camPos){
        SceneDataManager::SceneData data = sceneDataManager.makeScenedata(viewProj,camPos);
        sceneDataManager.setSceneData(data, currentIndex);
    }

    uint32_t uploadTexture(VmaAllocator allocator,Texture& texture);
    void     unloadTexture(uint32_t index);
    private:
        RenderGraph renderGraph;
        Swapchain swapchain;
        FrameManager frameManager;
        PipelineBuilder pipelineBuilder;
        ShaderLoader shaderLoader;
        // Variable de prueba, despues hay que cambiar y borrar
        vk::raii::Pipeline trianglePipeline{nullptr};
        vk::raii::PipelineLayout layout{nullptr};
        MeshBufferManager meshBufferManager;
        SceneDataManager sceneDataManager;
        MeshBufferManager::GpuMeshBuffers meshBuffers;
        uint32_t currentIndex = 0;
        AssetTexutureManager assetTextureManager;

        
};