#pragma once
#include <FrameManager.hpp>
#include <MeshBufferManager.hpp>
#include <cstdint>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan_raii.hpp>
#include "MeshBufferManager.hpp"

class SceneDataManager{
    public: 
        struct SceneData {
            //  Esta es una multiplicacion entre la matriz de projeccion y la matriz de vista
            glm::mat4 viewProj;
            glm::vec4 camPos;
        };

        struct FrameData {
            MeshBufferManager::GpuBuffer sceneBuffer;
            vk::DeviceAddress sceneAddress;
        };
        
    
    void init(MeshBufferManager& meshBuffers, const vk::raii::Device& device);
    SceneData makeScenedata(glm::mat4 viewProj,glm::vec4 camPos){
        SceneData data = {viewProj,camPos};
        return data;
    }
    
    void setSceneData(const SceneData& data, uint32_t frameIndex);

    vk::DeviceAddress getAdress(uint32_t frameIndex){
        return frames[frameIndex].sceneAddress;
    }

    MeshBufferManager::GpuBuffer getSceneBuffer(uint32_t frameIndex){
        return frames[frameIndex].sceneBuffer;
    }

    ~SceneDataManager() {
        for (auto& frame : frames) {
            manager->destroyBuffer(frame.sceneBuffer);
            frame.sceneAddress = 0;
        }
    }
    private:
        std::array<FrameData, MaxFramesInFlight> frames;
        MeshBufferManager* manager;

        
        
};