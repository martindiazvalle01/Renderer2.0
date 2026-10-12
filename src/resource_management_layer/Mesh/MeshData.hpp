// MeshData.hpp
#pragma once
#include <vector>
#include <memory>
#include <cstdint>
#include "Resource.hpp"
#include "Vertex.hpp"
#include "GltfSceneCache.hpp"

class ResourceManager;  // forward declaration — solo lo tenemos como referencia
class MaterialData;

// Un mesh puede tener varios "primitives" en glTF (submeshes con distinto
// material). Guardamos el rango de índices que le corresponde a cada uno
// dentro del buffer concatenado, más su material.

struct Submesh {
    uint32_t indexOffset = 0;
    uint32_t indexCount  = 0;
    std::shared_ptr<MaterialData> material; // puede ser nullptr si el primitive no tiene material asignado
};

class MeshData : public Resource {
    public:
        MeshData(const std::string& id, const std::string& filePath,
                size_t meshIndex, GltfSceneCache& sceneCache,
                ResourceManager& resourceManager, const std::string& basePath)
            : Resource(id), filePath(filePath), meshIndex(meshIndex), 
            sceneCache(sceneCache), resourceManager(resourceManager), basePath(basePath) {}

        const std::vector<Vertex>&   vertices()  const { return vertexData; }
        const std::vector<uint32_t>& indices()   const { return indexData; }
        const std::vector<Submesh>&  submeshes() const { return submeshList; }

    protected:
        bool doLoad() override;
        bool doUnload() override;

    private:
        std::string filePath;
        size_t meshIndex;
        GltfSceneCache& sceneCache;
        ResourceManager& resourceManager;
        std::string basePath;

        std::vector<Vertex>   vertexData;
        std::vector<uint32_t> indexData;
        std::vector<Submesh>  submeshList;


        void computeTangentsIfMissing();
};