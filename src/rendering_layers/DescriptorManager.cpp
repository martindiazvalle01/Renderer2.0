#include "DescriptorManager.hpp"
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

void DescriptorManager::init(const vk::raii::Device& device){
    this->device = &device;
    for(uint32_t i = 0; i < bindings.size(); i++){
        bindings.at(i).binding = i;
        bindings.at(i).descriptorType = types.at(i);
        //Este es simplemente el tamaño maximo que puede tener el array del layout, ya que no se cuantas voy a usar, solo colocare 1000
        //Gracias a la feature Partially_bound no tengo que elegir el numero exacto, el shader simplemente no los lee
        bindings.at(i).descriptorCount = Max_Textures;
        bindings.at(i).stageFlags = vk::ShaderStageFlagBits::eAll;
        flags.at(i) = vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind;
    }

    vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
    bindingFlags.sType = vk::StructureType::eDescriptorSetLayoutBindingFlagsCreateInfo;
    bindingFlags.pNext = nullptr;
    bindingFlags.pBindingFlags = flags.data();
    bindingFlags.bindingCount = bindings.size();

    vk::DescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eDescriptorSetLayoutCreateInfo;
    createInfo.bindingCount = bindings.size();
    createInfo.pBindings = bindings.data();
    //Esta solo se activa si el descriptorPool se actualiza despues de ser creado, aun no se si lo necesito, asi que queda desactivado
    createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;

    //Seteamos las binding flags
    createInfo.pNext = &bindingFlags;

    //Creamos el layout
    bindlessLayout = device.createDescriptorSetLayout(createInfo);

    vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eCombinedImageSampler, Max_Textures };

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind|
                  vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
            .setMaxSets(1)
            .setPoolSizes(poolSize);

    
    pool = device.createDescriptorPool(poolInfo);

    vk::DescriptorSetAllocateInfo allocateInfo{};
    allocateInfo.sType = vk::StructureType::eDescriptorSetAllocateInfo;
    allocateInfo.descriptorPool = *pool;
    allocateInfo.setSetLayouts(*bindlessLayout);

    allocateInfo.descriptorSetCount = 1;

    //Crea el descriptor
    auto sets = device.allocateDescriptorSets(allocateInfo);
    bindlessDescriptorSet = std::move(sets.front());

    
    
}

void DescriptorManager::storeTexture(uint32_t index,vk::ImageView imageView, vk::Sampler sampler){
  
    vk::DescriptorImageInfo imageInfo{};
    imageInfo.setSampler(sampler);
    imageInfo.setImageView(imageView);
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    vk::WriteDescriptorSet write{};
    write.setDstSet(bindlessDescriptorSet);
    write.setDstBinding(0);
    write.setDstArrayElement(index);
    write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    write.setImageInfo(imageInfo);
    device->updateDescriptorSets(write, nullptr);
    
}

