#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#include "Instance.hpp"

class DebugMessenger {
public:
    void create(const Instance& instance);
private:
    static VkBool32 debugCallback(
        vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        vk::DebugUtilsMessageTypeFlagsEXT messageType,
        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);


    vk::raii::DebugUtilsMessengerEXT messenger = nullptr;
};