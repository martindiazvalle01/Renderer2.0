#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

class Instance {
public:
    void create();
    const vk::raii::Instance& get() const { return instance; }
private:
    vk::raii::Context context;
    vk::raii::Instance instance = nullptr;
};
