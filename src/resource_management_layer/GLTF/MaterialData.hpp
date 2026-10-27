#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "TextureData.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include "ResourceManagerGLTF.hpp"

// Datos de material en CPU, extraídos directo de asset->materials[i] de fastgltf.

//Se crea porque da error si no se crea antes dellamar a la funcion FromGlth
class ResourceManagerGLTF;

enum class MaterialAlphaMode {
    Opaque,       // sin transparencia
    Mask,         // alpha test (cutoff)
    Blend         // transparencia real
};

struct MaterialData {

    static std::shared_ptr<MaterialData> FromGltf(
        const fastgltf::Asset& asset, 
        size_t materialIndex, 
        ResourceManagerGLTF& resourceManager,
        const std::string& basePath);

    static std::string ResolveImagePath(const fastgltf::Asset& asset, size_t imageIndex, const std::string& basePath);

    glm::vec4 baseColorFactor = glm::vec4(1.0f);
    float metallicFactor  = 1.0f;
    float roughnessFactor = 1.0f;
    glm::vec3 emissiveFactor = glm::vec3(0.0f);

    MaterialAlphaMode alphaMode = MaterialAlphaMode::Opaque;
    float alphaCutoff = 0.5f; // solo relevante si alphaMode es Mask

    // nullptr = el material no tiene esta textura asignada (usar textura blanca)
    std::shared_ptr<TextureData> baseColorTexture;
    std::shared_ptr<TextureData> metallicRoughnessTexture;
    std::shared_ptr<TextureData> normalTexture;
    std::shared_ptr<TextureData> occlusionTexture;
    std::shared_ptr<TextureData> emissiveTexture;
};