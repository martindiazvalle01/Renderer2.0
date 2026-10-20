#include "ShaderLoader.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

vk::raii::ShaderModule createShaderModule(const std::string& filename, const vk::raii::Device& device){
    std::cout << "Buscando shader en: " << std::filesystem::absolute(filename) << std::endl;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("No se pudo abrir " + filename);
 
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
 
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = buffer.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
 
    return vk::raii::ShaderModule(device, createInfo);
}

