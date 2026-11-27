#include "AssetManager.hpp"
#include <structTypes.hpp>
#include <utility>
#include <vector>

Handle<Model> AssetManager::loadModel(IntermediateStruct&& data){
    Model model;

    //Creacion de Texturas
    std::vector<Handle<Texture>> textures;
    textures.reserve(data.textures.size());

    for (auto& texture : data.textures){
        textures.push_back(GetOrCreateTexture(std::move(texture)));
    }

    //Creacion de Materiales

    std::vector<Handle<Material>> materials;

    for(auto& material : data.materials){
        materials.push_back(loadMaterial(material, textures));
    }

    model.materials.swap(materials);
    
    //Creacion de Meshes

    std::vector<Handle<Mesh>> meshes;
    meshes.reserve(data.meshes.size());

    for(auto& mesh : data.meshes){
        meshes.push_back(loadMesh(mesh));
    }


    model.meshes.swap(meshes);

    Handle<Model> out = modelPool.Create(std::move(model));
    return out;
}

Handle<Texture> AssetManager::GetOrCreateTexture(Texture&& data){

    // Si filePath está vacío (ej. textura embebida en GLB o fallback), evita colisionar en ""
    if (!data.filePath.empty()) {
        auto it = textureCache.find(data.filePath);
        if (it != textureCache.end()) {
            return it->second; // data se destruye limpio al salir y libera su ktxTex duplicado
        }
    }
    //Creamos una clave unica
    std::string key = data.filePath;
    Handle<Texture> out = texturePool.Create(std::move(data)); // Mueve ktxTex y deja data.ktxTex = nullptr
    
    // Revisamos si ya existe 
    auto it = textureCache.find(key);

    if(it != textureCache.end()){
        return it->second;
    }

    
    textureCache[key] = out;
    return out;
}

Handle<Material> AssetManager::loadMaterial(const MaterialData& materialData, std::vector<Handle<Texture>> textures){
    Material material;

    // Revisamos si la textura tiene indice
    if(materialData.textureIdx != -1){
        material.texture = textures[materialData.textureIdx];
    }
    if(materialData.normalIdx != -1){
        material.normal = textures[materialData.normalIdx];
    }
    if(materialData.metallicRoughnessIdx != -1){
        material.metallicRoughness = textures[materialData.metallicRoughnessIdx];
    }
    if(materialData.emissiveIdx != -1){
        material.emissive = textures[materialData.emissiveIdx];
    }
    if(materialData.occlusionIdx != -1){
        material.occlusion = textures[materialData.occlusionIdx];
    }

    material.baseColorFactor = materialData.baseColorFactor;
    material.metallicFactor = materialData.metallicFactor;
    material.roughnessFactor = materialData.roughnessFactor;
    material.emissiveFactor = materialData.emissiveFactor;
    material.emissiveStrength = materialData.emissiveStrength;
    material.occlusionStrength = materialData.occlusionStrength;
    material.normalScale      = materialData.normalScale;
    material.doubleSided     = materialData.doubleSided;
    material.alphaMode  = materialData.alphaMode;
    material.alphaCutoff     = materialData.alphaCutoff;


    Handle<Material> out = materialPool.Create(std::move(material));

    return out;
}


Handle<Mesh> AssetManager::loadMesh(Mesh meshData){
    Handle<Mesh> out = meshPool.Create(std::move(meshData));
    return out;
}