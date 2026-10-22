#include "ShaderLoader.hpp"
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

// Todo binario SPIR-V empieza con este número
static constexpr uint32_t SPIRV_MAGIC = 0x07230203;

vk::raii::ShaderModule ShaderLoader::createShaderModule(const std::string& filename, const vk::raii::Device& device){
    std::cout << "Buscando shader en: " << std::filesystem::absolute(filename) << std::endl;
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error("No se pudo abrir " + filename + " (¿se compilo el .slang?)");

    size_t fileSize = (size_t)file.tellg();
    if (fileSize == 0 || fileSize % 4 != 0)
        throw std::runtime_error("SPIR-V invalido (vacio o tamaño no multiplo de 4): " + filename);

    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize))
        throw std::runtime_error("Error leyendo " + filename);

    if (buffer[0] != SPIRV_MAGIC)
        throw std::runtime_error("El archivo no es SPIR-V (¿pasaste el .slang en vez del .spv?): " + filename);

    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.codeSize = fileSize;
    createInfo.pCode = buffer.data();

    return vk::raii::ShaderModule(device, createInfo);
}