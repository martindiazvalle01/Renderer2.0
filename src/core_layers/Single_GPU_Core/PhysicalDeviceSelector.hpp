#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "Instance.hpp"
#include "Surface.hpp"

class PhysicalDeviceSelector {
public:
    vk::raii::PhysicalDevice pick(const Instance& instance, const Surface& surface);
private:
    uint32_t scoreDevice(vk::raii::PhysicalDevice const& physicalDevice, const Surface& surface);
};