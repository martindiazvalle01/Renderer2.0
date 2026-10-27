#pragma once

#include <filesystem>
#include <algorithm>

enum class AssetFormat { GLTF, GLB, FBX, Unknown };

inline AssetFormat formatFromExtension(const std::filesystem::path& p) {
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == ".gltf") return AssetFormat::GLTF;
    if (ext == ".glb")  return AssetFormat::GLB;
    if (ext == ".fbx")  return AssetFormat::FBX;
    return AssetFormat::Unknown;
}