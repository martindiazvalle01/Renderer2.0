
#pragma once
#include <ktx.h>
#include <vulkan/vulkan.hpp>
#include <string>
#include "Resource.hpp"


//Esta clase solo tiene los datos de la imagen, sus datos crudos, el tamaño del archivo, 
//los mipmaps, etc, todo lo que es crear el sampler, la vkImage y el resto va en la capa de renderizado

class TextureData : public Resource {
public:
    // El id es la key lógica de cache ("brick", "player_diffuse", etc.)
    // filePath es la ruta real en disco al .ktx2
    TextureData(const std::string& id, const std::string& filePath)
        : Resource(id), filePath(filePath) {}

    ~TextureData() override { doUnload(); }

    ktxTexture2* raw() const { return ktxTex; }
    int width() const  { return ktxTex->baseWidth; }
    int height() const { return ktxTex->baseHeight; }
    vk::Format format() const { return static_cast<vk::Format>(ktxTex->vkFormat); }
    bool hasPregenMips() const { return ktxTex->numLevels > 1; }
    uint32_t fileMipLevels() const { return ktxTex->numLevels; }
    size_t dataSize() const { return ktxTexture_GetDataSize(ktxTexture(ktxTex)); }

    // No copiable (el puntero ktxTex es dueño de la memoria)
    TextureData(const TextureData&) = delete;
    TextureData& operator=(const TextureData&) = delete;

protected:
    // Estos dos son los que exige la interfaz Resource — se llaman
    // desde Load()/Unload() de la clase base
    bool doLoad() override;
    bool doUnload() override;

private:
    std::string filePath;
    ktxTexture2* ktxTex = nullptr;
};