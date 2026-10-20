#pragma once
#include <vulkan/vulkan_raii.hpp>

class ShaderLoader{

    public:
        vk::raii::ShaderModule createShaderModule(const std::string& filename, const vk::raii::Device& device);
};