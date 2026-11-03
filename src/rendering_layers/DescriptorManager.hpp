#pragma once
#include <FrameManager.hpp>
#include <MeshBufferManager.hpp>
#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
#include "MeshBufferManager.hpp"

class DescriptorManager{
    public: 
        struct SceneData {
            glm::mat4 viewProj;
            glm::vec4 camPos;
        };

        struct FrameData {
            MeshBufferManager::GpuBuffer sceneBuffer;
            vk::DeviceAddress sceneAddress;
        };
        
    void setSceneData();

    private:
        std::array<FrameData, MaxFramesInFlight> frames;
        
        
};