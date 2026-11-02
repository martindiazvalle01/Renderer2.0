#pragma once

#include "ktx.h"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>

// Todos los struct que se usaran en este layer

template<typename T>
struct Handle {
    uint32_t index  = 0; // Posicion del array
    uint32_t generation = 0; // version (Para detectar cuando el handle no es valido)
    
    bool IsValid() const {
        return index != 0;  // ya que uso uint32_t, voy a usar 0 para indicar que el handle no es valido
    }

    bool operator == (const Handle& other) const {
        return index == other.index && generation == other.generation;
    }

};


struct Vertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec2 uv0;
    glm::vec2 uv1;      //Opcional
    glm::vec4 color;    //Opcional

    //Skinning para cuando tenga soporte para animacion
    glm::uint16 boneIndices[4];
    glm::float32 boneWeights[4];
};


struct Submesh{
    glm::uint32_t firstIndex;
    glm::uint32   indexCount;
    int32_t baseVertex;         //El offset en el vertex buffer
    glm::uint32   materialIndex;
    // Despues podria colocar el bounding box local
};

struct Mesh {
    std::vector<Vertex>    vertices;
    std::vector<uint32_t>  indices;
    std::vector<Submesh>   submeshes;
    //  Aqui podria colocar el Bounding box 

    uint64_t               contentHash = 0; //Para evitar subir lo mismo 2 veces
};

struct Texture{
    // Aqui me pregunto que más podria tener, porque los datos para subirlos a la GPU estan en el rendering layer
    vk::Format format;
    uint32_t width, height, mips;
    std::string filePath;
    ktxTexture2* ktxTex = nullptr;
};

enum struct AlphaMode {
    Opaque,       // sin transparencia
    Mask,         // alpha test (cutoff)
    Blend         // transparencia real
    
};


struct Material{
    Handle<Texture> textura;
    Handle<Texture> normal;
    Handle<Texture> metallicRoughness;
    Handle<Texture> emissive;
    Handle<Texture> occlusion;

    glm::vec4 baseColorFactor = {1,1,1,1};
    float metallicFactor      = 1.0f;
    float roughnessFactor     = 1.0f;
    glm::vec3 emissiveFactor  = {0,0,0};
    float emissiveStrength    = 1.0f;
    float occlusionStrength   = 1.0f;
    float normalScale         = 1.0f;
    //despues ire viendo que otros faltan
    bool doubleSided         = false;
    AlphaMode alphaMode = AlphaMode::Opaque;
    float alphaCutoff   = 0;
};

struct Model{
    std::vector <Handle<Mesh>>  meshes;
    std::vector<Handle<Material>> materials;
};
// Esto es lo que le pasaremos a los importadores como contexto para que transformen el archivo a los struct model
struct FileContext{
    std::filesystem::path filePath;
    std::filesystem::path dirPath;
};

//Estos son los structs que usaran y retornaran los importer 

struct MaterialData{
    // -1 significa que no tiene ese tipo de textura
    int32_t textureIdx              = -1;
    int32_t normalIdx               = -1;
    int32_t metallicRoughnessIdx    = -1;
    int32_t emissiveIdx             = -1;
    int32_t occlusionIdx            = -1;

    glm::vec4 baseColorFactor = {1,1,1,1};
    float metallicFactor      = 1.0f;
    float roughnessFactor     = 1.0f;
    glm::vec3 emissiveFactor  = {0,0,0};
    float emissiveStrength    = 1.0f;
    float occlusionStrength   = 1.0f;
    float normalScale         = 1.0f;
    //despues ire viendo que otros faltan
    bool doubleSided         = false;
    AlphaMode alphaMode = AlphaMode::Opaque;
    float alphaCutoff   = 0;
};


struct IntermediateStruct{
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
    std::vector<MaterialData> materials;
};

#include <AssetPool.hpp>
// Este es el struct que contiene todos los asset pools
// Estan asi porque en el asset loader entran como parametros para crear el asset
struct AssetPools {
    AssetPool<Mesh>&      meshesPool;
    AssetPool<Material>&  materialsPool;
    AssetPool<Texture>&   texturesPool;
};
