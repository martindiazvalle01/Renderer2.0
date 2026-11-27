#pragma once
#include "DescriptorManager.hpp"
#include "GpuImage.hpp"
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <GpuImage.hpp>
#include <QueueSubmitter.hpp>
#include <cstdint>
#include <structTypes.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

// Clase que sube las texturas de los assets (KTX2 en RAM) a la GPU y las registra en el descriptor set bindless

class AssetTexutureManager{
    public:
    struct GpuTexture{
        GpuImage image;
        vk::raii::ImageView view = nullptr;
        vk::raii::Sampler sampler = nullptr;
        vk::Format format = vk::Format::eUndefined;
        vk::Extent2D extent{};
        uint32_t mipLevels = 0;
    };

    struct StagingBuffer{
        VmaAllocator allocator = nullptr;    
        VkBuffer stagingBuffer = VK_NULL_HANDLE;
        VmaAllocation stagingAllocation = nullptr;
        ~StagingBuffer() {
            if (stagingBuffer) vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
        }
    };

        void init(const vk::raii::Device& device, const vk::raii::Queue& queue, uint32_t queueFamilyIndex);
        
        uint32_t StoreTexture(VmaAllocator allocator,Texture& texture);
        void     UnloadTexture(uint32_t index);
        const vk::raii::DescriptorSetLayout& GetLayout() const { return descriptorManager.GetLayout(); }
        const vk::raii::DescriptorSet& GetDescriptorSet() const { return descriptorManager.GetDescriptorSet(); }
    private:
        GpuImage CreateImage(VmaAllocator allocator,const ktxTexture2* ktxTex);
        StagingBuffer createBuffer(VmaAllocator allocator,const ktxTexture2* ktxTex);
        // Barrier de Imagen en Synchronization2 sobre todos los mips
        void imageBarrier(vk::raii::CommandBuffer& cmd, vk::Image image, uint32_t mipLevels, vk::ImageLayout oldLayout, vk::ImageLayout newLayout
                         , vk::PipelineStageFlagBits2 srcStage, vk::AccessFlagBits2 srcAcess, vk::PipelineStageFlags2 dstStage, vk::AccessFlagBits2 dstAcess);

        const vk::raii::Device* device;
        std::vector<GpuTexture> textures;
        std::vector<uint32_t> freeSlots;
        vk::raii::CommandPool uploadPool = nullptr;
        uint32_t nextSlot = 0;

        DescriptorManager descriptorManager;
        QueueSubmitter submitter;
};