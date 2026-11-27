#include "AssetTextureManager.hpp"
#include "ktx.h"
#include "spdlog/spdlog.h"
#include "vulkan/vulkan.hpp"
#include <DescriptorManager.hpp>
#include <GpuImage.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <structTypes.hpp>
#include <utility>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

void AssetTexutureManager::init(const vk::raii::Device& device, const vk::raii::Queue& queue, uint32_t queueFamilyIndex){
    descriptorManager.init(device);
    submitter.init(device, queue);
    this->device = &device;

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::CommandPoolCreateFlagBits::eTransient);
    poolInfo.setQueueFamilyIndex(queueFamilyIndex);
    

    uploadPool = vk::raii::CommandPool(device,poolInfo);

    // Colocamos como el maximo de texturas Max_textures
    textures.resize(Max_Textures);
    
}

// Barrier de Imagen en Synchronization2 sobre todos los mips
void AssetTexutureManager::imageBarrier(vk::raii::CommandBuffer& cmd, vk::Image image, uint32_t mipLevels, vk::ImageLayout oldLayout, vk::ImageLayout newLayout
                         , vk::PipelineStageFlagBits2 srcStage, vk::AccessFlagBits2 srcAcess, vk::PipelineStageFlags2 dstStage, vk::AccessFlagBits2 dstAcess){


    vk::ImageMemoryBarrier2 barrier{};
    barrier.setSrcStageMask(srcStage);
    barrier.setSrcAccessMask(srcAcess);
    barrier.setDstStageMask(dstStage);
    barrier.setDstAccessMask(dstAcess);
    barrier.setOldLayout(oldLayout);
    barrier.setNewLayout(newLayout);
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
    barrier.setImage(image);
    barrier.setSubresourceRange({vk::ImageAspectFlagBits::eColor,0,mipLevels,0,1});

    vk::DependencyInfo dep{};
    dep.setImageMemoryBarriers(barrier);
    cmd.pipelineBarrier2(dep);
    }


uint32_t AssetTexutureManager::StoreTexture(VmaAllocator allocator,Texture& texture){
    ktxTexture2* ktxTex = texture.ktxTex;

    GpuImage image = CreateImage(allocator,ktxTex);
    StagingBuffer staging = createBuffer(allocator, ktxTex);

    
    const VkImage vkImage = image.get();
    const vk::Buffer vkStaging = staging.stagingBuffer;
    // Regiones = multiples imagenes copia de la textura, las cuales se usan para optimizar, asi colocando texturas de menor resolucion 
    std::vector<vk::BufferImageCopy> regions;
    regions.reserve(ktxTex->numLevels);
    for(uint32_t level = 0; level < ktxTex->numLevels; ++level){
        ktx_size_t offset = 0;
        if(ktxTexture_GetImageOffset(ktxTexture(ktxTex), level, 0, 0, &offset) != KTX_SUCCESS){
            throw std::runtime_error("Store Texture: No se pudo obtener el offset del mip");
        }
        vk::BufferImageCopy region;
        region.setBufferOffset(offset);
        region.setBufferRowLength(0);
        region.setBufferImageHeight(0);
        region.setImageSubresource({vk::ImageAspectFlagBits::eColor,level,0,1});
        // >> level es un desplazamiento de bits hacia la derecha, pasa los bits de la pantalla de por ejemplo: 1024->512->256->128, asi hasta llegar a 0 o que se acaben las regiones.
        region.setImageExtent({std::max(1u,ktxTex->baseWidth >> level),std::max(1u, ktxTex->baseHeight >> level),1 });
        regions.push_back(region);
    }
    // Subimos el command Buffer al command pool usando el submitter y usando barreras para que no se rompa la sincronizacion
    submitter.submitAndWait(uploadPool, [&] (vk::raii::CommandBuffer& cmd){
        imageBarrier(cmd, vkImage, ktxTex->numLevels, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 
                     vk::PipelineStageFlagBits2::eNone, vk::AccessFlagBits2::eNone, vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite);

        cmd.copyBufferToImage(vkStaging, vkImage, vk::ImageLayout::eTransferDstOptimal, regions);

        vk::PipelineStageFlags2 shaderStages =
              vk::PipelineStageFlagBits2::eVertexShader
            | vk::PipelineStageFlagBits2::eFragmentShader
            | vk::PipelineStageFlagBits2::eComputeShader;

        imageBarrier(cmd, vkImage, ktxTex->numLevels,
                     vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal,
                     vk::PipelineStageFlagBits2::eCopy, vk::AccessFlagBits2::eTransferWrite,
                     shaderStages,
                     vk::AccessFlagBits2::eShaderSampledRead);
    });

    //View
    vk::Format format = static_cast<vk::Format>(ktxTex->vkFormat);
    if(format == vk::Format::eUndefined){
        throw std::runtime_error("StoreTexture: FormatoVulkan Indefinido en el KTX");
    }  

    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.setImage(vkImage);
    viewInfo.setViewType(vk::ImageViewType::e2D);
    viewInfo.setFormat(format);
    viewInfo.setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0 , ktxTex->numLevels,0,1});

    vk::raii::ImageView view(*device,viewInfo);
    
    // Sampler
    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.setMagFilter(vk::Filter::eLinear);
    samplerInfo.setMinFilter(vk::Filter::eLinear);
    samplerInfo.setMipmapMode(vk::SamplerMipmapMode::eLinear);
    samplerInfo.setAddressModeU(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeV(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setAddressModeW(vk::SamplerAddressMode::eRepeat);
    samplerInfo.setMinLod(0.0f);
    samplerInfo.setMaxLod(VK_LOD_CLAMP_NONE);
    samplerInfo.setBorderColor(vk::BorderColor::eIntOpaqueBlack);

    vk::raii::Sampler sampler(*device,samplerInfo);

    // Elegir el slot

    uint32_t index;
    if(!freeSlots.empty()){
        index = freeSlots.back();
        freeSlots.pop_back();
    }else{
        if(nextSlot >= Max_Textures){
            throw std::runtime_error("StoreTexture: Se alcanzo el maximo de texturas disponibles");
        }
        index = nextSlot++;
    }

    //Guardamos la textura

    GpuTexture& slot = textures[index];
    slot.image = std::move(image);
    slot.view  = std::move(view);
    slot.sampler = std::move(sampler);
    slot.format = format;
    slot.extent = vk::Extent2D{ktxTex->baseWidth,ktxTex->baseHeight};
    slot.mipLevels = ktxTex->numLevels;

    //Escribimos el descriptor en ese slot
    descriptorManager.storeTexture(index, *slot.view, slot.sampler);

  

    return index;
}

GpuImage AssetTexutureManager::CreateImage(VmaAllocator allocator,const ktxTexture2* ktxTex){
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent        = {ktxTex->baseWidth, ktxTex->baseHeight, ktxTex->baseDepth };
    imageInfo.mipLevels     = ktxTex->numLevels;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = static_cast<VkFormat>(ktxTex->vkFormat) ;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo imgAllocInfo{};
    imgAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    GpuImage image = GpuImage(allocator, imageInfo, imgAllocInfo);

    return image;
    
}

AssetTexutureManager::StagingBuffer AssetTexutureManager::createBuffer(VmaAllocator allocator,const ktxTexture2* ktxTex){
    StagingBuffer buffer;
    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | 
                       VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = ktxTex->dataSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocation stagingAlloc;
    VkBuffer stagingBuffer;

    vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &stagingBuffer, &stagingAlloc, nullptr);

    void* data;
    vmaMapMemory(allocator, stagingAlloc, &data);

    spdlog::info("UPLOAD: tex={} pData={} dataSize={}",
    (void*)ktxTex, (void*)ktxTex->pData, ktxTex->dataSize);
    if (!ktxTex)
        throw std::runtime_error("ktxTex es nullptr (la textura nunca se cargó)");
    if (!ktxTex->pData || ktxTex->dataSize == 0)
        throw std::runtime_error("KTX sin datos de imagen");

    memcpy(data, ktxTex->pData, ktxTex->dataSize);

    vmaUnmapMemory(allocator, stagingAlloc);

    buffer.stagingAllocation = stagingAlloc;
    buffer.stagingBuffer = stagingBuffer;
    buffer.allocator = allocator;

    return buffer;

}

void AssetTexutureManager::UnloadTexture(uint32_t index){
    assert(index < Max_Textures);
    GpuTexture& t = textures[index];
    if(!t.image.get()) return; // el slot ya esta vacio

    device->waitIdle();

    // Destruir view y sampler antes de la Imagen
    t.view      = nullptr;
    t.sampler   = nullptr;
    t.image     = GpuImage{};
    t.format    = vk::Format::eUndefined;
    t.extent    = vk::Extent2D{};
    t.mipLevels = 0;

    freeSlots.push_back(index);
}