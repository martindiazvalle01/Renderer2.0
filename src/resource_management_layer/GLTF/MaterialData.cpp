
#include "MaterialData.hpp"
#include <TextureData.hpp>
#include <spdlog/spdlog.h>

std::shared_ptr<MaterialData> MaterialData::FromGltf(
    const fastgltf::Asset& asset, size_t materialIndex, 
    ResourceManagerGLTF& resourceManager, const std::string& basePath)
{
    const fastgltf::Material& gltfMat = asset.materials[materialIndex];
    auto matData = std::make_shared<MaterialData>();

    auto& pbr = gltfMat.pbrData;
    matData->baseColorFactor = glm::vec4(pbr.baseColorFactor[0], pbr.baseColorFactor[1],
                                          pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
    matData->metallicFactor  = pbr.metallicFactor;
    matData->roughnessFactor = pbr.roughnessFactor;

    if (pbr.baseColorTexture.has_value()) {
        size_t texIdx = pbr.baseColorTexture->textureIndex;
        size_t imgIdx = asset.textures[texIdx].imageIndex.value();
        std::string texPath = ResolveImagePath(asset, imgIdx, basePath);
        //El Primer parametro de load es el ID y el segundo el texture path
        //Posible TODO: crear un sistema de Identificacion sin usar simplemente el texPath
        matData->baseColorTexture = resourceManager.load<TextureData>(texPath, texPath);
        if (!matData->baseColorTexture) {
            spdlog::error("Fallo al cargar baseColorTexture para material {}", materialIndex);
        }
    }
    if(pbr.metallicRoughnessTexture.has_value())  {
        size_t texIdx = pbr.metallicRoughnessTexture->textureIndex;
        size_t imgIdx = asset.textures[texIdx].imageIndex.value();
        std::string texPath = ResolveImagePath(asset,imgIdx,basePath);

        matData->metallicRoughnessTexture = resourceManager.load<TextureData>(texPath,texPath);
        if(!matData->metallicRoughnessTexture){
            spdlog::error("Fallo al cargar metallicRoughnessTexture para material {}", materialIndex);
        }
    }

    matData->emissiveFactor = glm::vec3(gltfMat.emissiveFactor[0], gltfMat.emissiveFactor[1], gltfMat.emissiveFactor[2]);

    if (gltfMat.emissiveTexture.has_value()) {
        size_t texIdx = gltfMat.emissiveTexture->textureIndex;
        size_t imgIdx = asset.textures[texIdx].imageIndex.value();
        std::string texPath = ResolveImagePath(asset, imgIdx, basePath);
        matData->emissiveTexture = resourceManager.load<TextureData>(texPath, texPath);
        if(!matData->emissiveTexture){
            spdlog::error("Fallo al cargar emissiveTexture para material {}", materialIndex);
        }
    }

    if (gltfMat.normalTexture.has_value()) {
        size_t texIdx = gltfMat.normalTexture->textureIndex;
        size_t imgIdx = asset.textures[texIdx].imageIndex.value();
        std::string texPath = ResolveImagePath(asset, imgIdx, basePath);
        matData->normalTexture = resourceManager.load<TextureData>(texPath, texPath);
        if(!matData->normalTexture){
            spdlog::error("Fallo al cargar normalTexture para material {}", materialIndex);
        }
    }

    if (gltfMat.occlusionTexture.has_value()) {
        size_t texIdx = gltfMat.occlusionTexture->textureIndex;
        size_t imgIdx = asset.textures[texIdx].imageIndex.value();
        std::string texPath = ResolveImagePath(asset, imgIdx, basePath);
        matData->occlusionTexture = resourceManager.load<TextureData>(texPath, texPath);
        if(!matData->occlusionTexture){
            spdlog::error("Fallo al cargar occlusionTexture para material {}", materialIndex);
        }
    }
    

    switch (gltfMat.alphaMode) {
        case fastgltf::AlphaMode::Opaque: matData->alphaMode = MaterialAlphaMode::Opaque; break;
        case fastgltf::AlphaMode::Mask:   matData->alphaMode = MaterialAlphaMode::Mask; break;
        case fastgltf::AlphaMode::Blend:  matData->alphaMode = MaterialAlphaMode::Blend; break;
    }
    matData->alphaCutoff = gltfMat.alphaCutoff;

    return matData;
}


std::string MaterialData::ResolveImagePath(const fastgltf::Asset& asset, size_t imageIndex, const std::string& basePath) {
    const fastgltf::Image& image = asset.images[imageIndex];

    auto* uri = std::get_if<fastgltf::sources::URI>(&image.data);
    if (!uri) {
        // No debería pasar con los assets actuales (todos usan textura suelta)
        // pero, por si acaso, algun modelo se cree especial por hacer la imagen embebida, se queda
        spdlog::error("Imagen embebida sin soportar todavía — imageIndex {}", imageIndex);
        return "";
    }

    std::filesystem::path fullPath = std::filesystem::path(basePath) / uri->uri.fspath();
    fullPath.replace_extension(".ktx2"); // Recuerda: Para que esto funcione hay que crear el batch que transforma las imagenes al formato .ktx
    return fullPath.string();
}