#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "structTypes.hpp"

// Esta es simplemente una clase hecha con IA para probar que que se puedan subir datos en el rendering layer



class Cube {
public:
    // Genera un cubo centrado en el origen
    // size = longitud del lado
    explicit Cube(float size = 1.0f) {
        generate(size);
    }

    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<uint32_t>& getIndices() const { return m_indices; }

    // Si querés regenerarlo con otro tamaño
    void setSize(float size) {
        generate(size);
    }

private:
    std::vector<Vertex>  m_vertices;
    std::vector<uint32_t> m_indices;

    void generate(float size) {
        m_vertices.clear();
        m_indices.clear();

        const float hs = size * 0.5f; // half size

        // 6 caras × 4 vértices = 24 vértices (necesario para normales y UVs correctos)
        // Orden de las caras: +X, -X, +Y, -Y, +Z, -Z

        // --- +X (derecha) ---
        addFace(
            { hs, -hs, -hs}, { hs,  hs, -hs}, { hs,  hs,  hs}, { hs, -hs,  hs},
            { 1, 0, 0},
            {0,0}, {0,1}, {1,1}, {1,0}
        );

        // --- -X (izquierda) ---
        addFace(
            {-hs, -hs,  hs}, {-hs,  hs,  hs}, {-hs,  hs, -hs}, {-hs, -hs, -hs},
            {-1, 0, 0},
            {0,0}, {0,1}, {1,1}, {1,0}
        );

        // --- +Y (arriba) ---
        addFace(
            {-hs,  hs, -hs}, {-hs,  hs,  hs}, { hs,  hs,  hs}, { hs,  hs, -hs},
            { 0, 1, 0},
            {0,0}, {0,1}, {1,1}, {1,0}
        );

        // --- -Y (abajo) ---
        addFace(
            {-hs, -hs,  hs}, {-hs, -hs, -hs}, { hs, -hs, -hs}, { hs, -hs,  hs},
            { 0,-1, 0},
            {0,0}, {0,1}, {1,1}, {1,0}
        );

        // --- +Z (frente) ---
        addFace(
            {-hs, -hs,  hs}, { hs, -hs,  hs}, { hs,  hs,  hs}, {-hs,  hs,  hs},
            { 0, 0, 1},
            {0,0}, {1,0}, {1,1}, {0,1}
        );

        // --- -Z (atrás) ---
        addFace(
            { hs, -hs, -hs}, {-hs, -hs, -hs}, {-hs,  hs, -hs}, { hs,  hs, -hs},
            { 0, 0,-1},
            {0,0}, {1,0}, {1,1}, {0,1}
        );
    }

    void addFace(const glm::vec3& v0, const glm::vec3& v1,
                 const glm::vec3& v2, const glm::vec3& v3,
                 const glm::vec3& normal,
                 const glm::vec2& uv0, const glm::vec2& uv1,
                 const glm::vec2& uv2, const glm::vec2& uv3)
    {
        // Calculamos el tangente (para normal mapping)
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v3 - v0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv3 - uv0;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent = glm::normalize(tangent);

        // Handedness (w del tangent)
        glm::vec3 bitangent = glm::cross(normal, tangent);
        float handedness = (glm::dot(bitangent, glm::cross(edge1, edge2)) < 0.0f) ? -1.0f : 1.0f;

        uint32_t baseIndex = static_cast<uint32_t>(m_vertices.size());

        auto makeVertex = [&](const glm::vec3& pos, const glm::vec2& uv) -> Vertex {
            Vertex v{};
            v.position = pos;
            v.normal   = normal;
            v.tangent  = glm::vec4(tangent, handedness);
            v.uv0      = uv;
            v.uv1      = uv;                    // por defecto igual a uv0
            v.color    = glm::vec4(1.0f);       // blanco por defecto

            // Skinning en 0 (sin animación)
            v.boneIndices[0] = v.boneIndices[1] = v.boneIndices[2] = v.boneIndices[3] = 0;
            v.boneWeights[0] = 1.0f;
            v.boneWeights[1] = v.boneWeights[2] = v.boneWeights[3] = 0.0f;

            return v;
        };

        m_vertices.push_back(makeVertex(v0, uv0));
        m_vertices.push_back(makeVertex(v1, uv1));
        m_vertices.push_back(makeVertex(v2, uv2));
        m_vertices.push_back(makeVertex(v3, uv3));

        // Dos triángulos
        m_indices.push_back(baseIndex + 0);
        m_indices.push_back(baseIndex + 1);
        m_indices.push_back(baseIndex + 2);

        m_indices.push_back(baseIndex + 0);
        m_indices.push_back(baseIndex + 2);
        m_indices.push_back(baseIndex + 3);
    }
};