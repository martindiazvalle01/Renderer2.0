#pragma once
#include <glm/fwd.hpp>
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>
#include <Allocator.hpp>
#include <ResourceLayer/structTypes.hpp>
#include "QueueSubmitter.hpp"

class MeshBufferManager{
    public:
        struct GpuBuffer{
            vk::Buffer buffer = nullptr;
            VmaAllocation allocation = nullptr;
            VmaAllocationInfo info{};
        };

        // Contiene los recursos necesarios para un mesh
        struct GpuMeshBuffers {
            GpuBuffer indexBuffer;
            GpuBuffer vertexBuffer;
            vk::DeviceAddress vertexBufferAddress = 0;
            uint32_t indexCount = 0;
        };

        GpuBuffer createBuffer(uint64_t allocsize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,VmaAllocationCreateFlags flags = 0);

        void destroyBuffer(GpuBuffer& buffer);

        MeshBufferManager(VmaAllocator allocator, const vk::raii::Device& device,
                  const vk::raii::Queue& queue, uint32_t queueFamilyIndex);

        
        GpuMeshBuffers uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices);
    private:
        VmaAllocator allocator;
        vk::raii::CommandPool uploadPool = nullptr;
        const vk::raii::Device& device;
        const vk::raii::Queue&  queue;
        QueueSubmitter queueSubmitter;
        
};