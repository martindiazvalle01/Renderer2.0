#include "AssetManager.hpp"
#include <structTypes.hpp>
#include <utility>
#include <vector>

Handle<Model> AssetManager::loadModel(const IntermediateStruct& data){
    Model model;

    //Creacion de Texturas
    std::vector<Handle<Texture>> textures;
    textures.reserve(data.textures.size());

    for (auto& texture : data.textures){
        textures.push_back(GetOrCreateTexture(texture));
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
        meshes.push_back(loadMesh(std::move(mesh)));
    }


    Handle<Model> out = modelPool.Create(std::move(model));
    return out;
}

Handle<Texture> AssetManager::GetOrCreateTexture(const Texture& data){
    //Creamos una clave unica

    std::string key = data.filePath;
    
    // Revisamos si ya existe 
    auto it = textureCache.find(key);

    if(it != textureCache.end()){
        return it->second;
    }
    // Tengo que crear esto porque el const da error en el std::move y el for each siempre retorna la variable con un const
    Texture texture;
    texture.format = data.format;
    texture.width  = data.width;
    texture.height = data.height;
    texture.mips   = data.mips;
    texture.filePath = data.filePath;
    texture.ktxTex = data.ktxTex;
    // Si no existe, la creamos

    Handle<Texture> out = texturePool.Create(std::move(texture));
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