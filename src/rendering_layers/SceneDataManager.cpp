#include "SceneDataManager.hpp"
#include "vulkan/vulkan.hpp"
#include <FrameManager.hpp>
#include <MeshBufferManager.hpp>
#include <cstdint>
#include <cstring>


void SceneDataManager::init(MeshBufferManager& meshBuffers, const vk::raii::Device& device){
    manager = &meshBuffers;
    for(uint32_t i = 0; i < MaxFramesInFlight; i++){
        //Aqui deberia crear el buffer
        frames[i].sceneBuffer = meshBuffers.createBuffer(sizeof(SceneData), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,  
                                            VMA_MEMORY_USAGE_CPU_TO_GPU, VMA_ALLOCATION_CREATE_MAPPED_BIT);

        vk::BufferDeviceAddressInfo info;
        info.setBuffer(frames[i].sceneBuffer.buffer);
        frames[i].sceneAddress = device.getBufferAddress(info);
    }
    
}


void SceneDataManager::setSceneData(const SceneDataManager::SceneData& data, uint32_t frameIndex){
    memcpy(frames[frameIndex].sceneBuffer.info.pMappedData, &data, sizeof(SceneData));
}

