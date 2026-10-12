#pragma once
#include <unordered_map>
#include <memory>
#include <string>
#include <filesystem>
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <spdlog/spdlog.h>


class GltfSceneCache {
    public:
        std::shared_ptr<const fastgltf::Asset> get(const std::string& filePath) {
        //Esta cosa es simplmente para diferenciar archivos si tienen el mismo nombre
        std::string canonicalPath = std::filesystem::weakly_canonical(filePath).string();

        auto it = cache.find(canonicalPath);
        if (it != cache.end()) {
            return it->second;
        }

        auto asset = load(canonicalPath);
        if (!asset) return nullptr;

        cache[canonicalPath] = asset;
        return asset;
}

    private:
        std::shared_ptr<fastgltf::Asset> load(const std::string& filePath) {
            std::filesystem::path path(filePath);

            auto dataResult = fastgltf::GltfDataBuffer::FromPath(path);
            if (dataResult.error() != fastgltf::Error::None) {
                spdlog::error("Fallo al cargar buffer de {}: {}", filePath, 
                fastgltf::getErrorMessage(dataResult.error()));
                return nullptr;
}

            // Necesario para poder resolver rutas relativas a buffers/imágenes
            // externas referenciadas dentro del .gltf
            auto basePath = path.parent_path();

            fastgltf::Parser parser;
            constexpr auto options =
                fastgltf::Options::LoadExternalBuffers |
                fastgltf::Options::GenerateMeshIndices;

            auto assetResult = parser.loadGltf(dataResult.get(), basePath, options);
            if (assetResult.error() != fastgltf::Error::None) {
                return nullptr;
            }

            return std::make_shared<fastgltf::Asset>(std::move(assetResult.get()));
        }

        std::unordered_map<std::string, std::shared_ptr<fastgltf::Asset>> cache;
};