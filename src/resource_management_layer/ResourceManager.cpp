
#include "ResourceManager.hpp"
#include "MaterialData.hpp"  

std::shared_ptr<MaterialData> ResourceManager::loadMaterialFromGltf(
    const fastgltf::Asset& asset, size_t materialIndex, const std::string& basePath)
{
    std::string cacheKey = basePath + "#material" + std::to_string(materialIndex);

    auto it = materialCache.find(cacheKey);
    if (it != materialCache.end()) {
        return it->second;
    }

    auto matData = MaterialData::FromGltf(asset, materialIndex, *this, basePath);
    materialCache[cacheKey] = matData;
    return matData;
}