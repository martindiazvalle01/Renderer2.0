// MeshData.cpp
#include "MeshData.hpp"
#include "MaterialData.hpp"
#include "ResourceManager.hpp"
#include <fastgltf/tools.hpp>

bool MeshData::doLoad() {
    auto asset = sceneCache.get(filePath);
    if (!asset || meshIndex >= asset->meshes.size()) {
        return false;
    }

    const fastgltf::Mesh& mesh = asset->meshes[meshIndex];

    for (const auto& primitive : mesh.primitives) {
        uint32_t vertexOffset = static_cast<uint32_t>(vertexData.size());
        uint32_t indexOffset  = static_cast<uint32_t>(indexData.size()); 

        // Posiciones 
        auto* positionIt = primitive.findAttribute("POSITION");
        if (positionIt == primitive.attributes.end()) continue;

        auto& posAccessor = asset->accessors[positionIt->accessorIndex];
        vertexData.resize(vertexData.size() + posAccessor.count);

        fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
            *asset, posAccessor,
            [&](fastgltf::math::fvec3 pos, size_t idx) {
                vertexData[vertexOffset + idx].position = { pos.x(), pos.y(), pos.z() };
            });

        // Normales 
        auto* normalIt = primitive.findAttribute("NORMAL");
        if (normalIt != primitive.attributes.end()) {
            auto& normAccessor = asset->accessors[normalIt->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
                *asset, normAccessor,
                [&](fastgltf::math::fvec3 n, size_t idx) {
                    vertexData[vertexOffset + idx].normal = { n.x(), n.y(), n.z() };
                });
        }

        // UVs 
        auto* uvIt = primitive.findAttribute("TEXCOORD_0");
        if (uvIt != primitive.attributes.end()) {
            auto& uvAccessor = asset->accessors[uvIt->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(
                *asset, uvAccessor,
                [&](fastgltf::math::fvec2 uv, size_t idx) {
                    vertexData[vertexOffset + idx].uv = { uv.x(), uv.y() };
                });
        }

        // Indices 
        if (primitive.indicesAccessor.has_value()) {
            auto& idxAccessor = asset->accessors[primitive.indicesAccessor.value()];
            fastgltf::iterateAccessor<uint32_t>(*asset, idxAccessor,
                [&](uint32_t idx) {
                    indexData.push_back(vertexOffset + idx);
                });
        }
        auto* tangentIt = primitive.findAttribute("TANGENT");
        spdlog::info("TANGENT encontrado: {}", tangentIt != primitive.attributes.end());
        if (tangentIt != primitive.attributes.end()) {
            auto& tanAccessor = asset->accessors[tangentIt->accessorIndex];
            fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec4>(
                *asset, tanAccessor,
                [&](fastgltf::math::fvec4 t, size_t idx) {
            vertexData[vertexOffset + idx].tangent = { t.x(), t.y(), t.z(), t.w() };
        });
        }

        uint32_t indexCount = static_cast<uint32_t>(indexData.size()) - indexOffset;

        
        std::shared_ptr<MaterialData> material;
        if (primitive.materialIndex.has_value()) {
            material = MaterialData::FromGltf(*asset, primitive.materialIndex.value(), 
                                               resourceManager, basePath);
        }
        // si no tiene materialIndex, queda nullptr — GpuMaterial usará un default

        submeshList.push_back(Submesh{indexOffset, indexCount, material});
    }

    computeTangentsIfMissing();

    return !vertexData.empty();
}

bool MeshData::doUnload() {
    vertexData.clear();
    indexData.clear();
    submeshList.clear();
    return true;
}

//Esta cosa la Robe, no tengo ni idea como calcula las tangentes faltantes
void MeshData::computeTangentsIfMissing() {
    // Ya viene con TANGENT del archivo — no hay nada que hacer
    bool hasTangents = std::any_of(vertexData.begin(), vertexData.end(),
        [](const Vertex& v) { return v.tangent != glm::vec4(0.0f); });
    if (hasTangents) return;

    std::vector<glm::vec3> tan1(vertexData.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> tan2(vertexData.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < indexData.size(); i += 3) {
        uint32_t i0 = indexData[i], i1 = indexData[i + 1], i2 = indexData[i + 2];

        const glm::vec3& p0 = vertexData[i0].position;
        const glm::vec3& p1 = vertexData[i1].position;
        const glm::vec3& p2 = vertexData[i2].position;

        const glm::vec2& uv0 = vertexData[i0].uv;
        const glm::vec2& uv1 = vertexData[i1].uv;
        const glm::vec2& uv2 = vertexData[i2].uv;

        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (std::abs(det) < 1e-8f) continue; // UVs degeneradas, saltar este triángulo

        float r = 1.0f / det;
        glm::vec3 tangent   = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
        glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;

        tan1[i0] += tangent; tan1[i1] += tangent; tan1[i2] += tangent;
        tan2[i0] += bitangent; tan2[i1] += bitangent; tan2[i2] += bitangent;
    }

    for (size_t i = 0; i < vertexData.size(); ++i) {
        const glm::vec3& n = vertexData[i].normal;
        const glm::vec3& t = tan1[i];

        // Ortogonalización de Gram-Schmidt contra la normal
        glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));

        // Signo de la bitangente (w), para reconstruirla en el shader
        float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;

        vertexData[i].tangent = glm::vec4(tangent, w);
    }
}