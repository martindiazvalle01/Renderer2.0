#pragma once
#include <string>
#include <vulkan/vulkan_raii.hpp>

class ShaderLoader{

    public:
        static vk::raii::ShaderModule createShaderModule(const std::string& filename, const vk::raii::Device& device);
};