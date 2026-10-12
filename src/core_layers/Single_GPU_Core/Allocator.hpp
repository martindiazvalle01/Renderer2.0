#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "vk_mem_alloc.h"
#include "Instance.hpp"
#include "LogicalDevice.hpp"


class Allocator {
public:
    void create(const Instance& instance, const vk::raii::PhysicalDevice& physicalDevice, const LogicalDevice& logicalDevice);
    VmaAllocator get() const { return allocator; }
private:
    VmaAllocator allocator = nullptr;
};
