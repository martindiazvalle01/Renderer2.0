#define VMA_IMPLEMENTATION
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"



#include "Allocator.hpp"
#include "Instance.hpp"
#include "LogicalDevice.hpp"

void Allocator::create(const Instance& instance, const vk::raii::PhysicalDevice& physicalDevice, const LogicalDevice& device){

    VmaVulkanFunctions vmaVulkanFunctions{};
    vmaVulkanFunctions.vkGetInstanceProcAddr = instance.get().getDispatcher()->vkGetInstanceProcAddr;
    vmaVulkanFunctions.vkGetDeviceProcAddr   = device.get().getDispatcher()->vkGetDeviceProcAddr;

    //Creacion del allocator para VMA
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.instance =        *instance.get();
    allocatorInfo.physicalDevice =  *physicalDevice;
    allocatorInfo.device =          *device.get();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;
    allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
    //Flag para poder usar Buffer device address
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    
    vmaCreateAllocator(&allocatorInfo, &allocator);
}
 