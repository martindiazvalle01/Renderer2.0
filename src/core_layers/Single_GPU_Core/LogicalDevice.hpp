#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "Surface.hpp"


class LogicalDevice {
public:
    void create(const vk::raii::PhysicalDevice& physicalDevice, const Surface& surface);
    const vk::raii::Device& get() const { return device; }
    const vk::raii::Queue& getGraphicsQueue() const { return graphicsQueue; }
    uint32_t getGraphicsIndex() const { return graphicsIndex; }
    uint32_t getPresentIndex() const { return presentIndex; }
private:

    void findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const Surface& surface);

    vk::raii::Device device = nullptr;
    vk::raii::Queue graphicsQueue = nullptr;
    uint32_t graphicsIndex = UINT32_MAX;
    uint32_t presentIndex = UINT32_MAX;
};
