#include <vulkan/vulkan.h>
#include <iostream>
#include <vulkan/vulkan_raii.hpp>
#include "Instance.hpp"
#include "DebugMessenger.hpp"

VkBool32 DebugMessenger::debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT      /*messageSeverity*/,
    vk::DebugUtilsMessageTypeFlagsEXT             /*messageType*/,
    const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                         /*pUserData*/)
{
    std::cerr << "Validation: " << pCallbackData->pMessage << "\n";
    return VK_FALSE;
}
 
void DebugMessenger::create(const Instance& instance) {
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{
        {},
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        &debugCallback
    };
 
    messenger = instance.get().createDebugUtilsMessengerEXT(createInfo);
}