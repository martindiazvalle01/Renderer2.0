
#include "GltfImporter.hpp"
#include "fastgltf/types.hpp"
#include <cstddef>
#include <structTypes.hpp>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <fastgltf/tools.hpp>



std::vector<MaterialData>  GltfImporter::loadMaterials(fastgltf::Asset& asset){
    std::vector<MaterialData> materials;

    for(auto& m : asset.materials){
        materials.push_back(loadMaterial(m));
    }
    
    return materials;
    
}

MaterialData GltfImporter::loadMaterial(const fastgltf::Material& material) {
    MaterialData out;

    // indices de texturas (Tienen el if a veces no vienen las texturas y dan error si asigno las variables a algo que no existe)
    if (material.pbrData.baseColorTexture.has_value()) {
        out.textureIdx = static_cast<int32_t>(material.pbrData.baseColorTexture->textureIndex);
    }

    if (material.normalTexture.has_value()) {
        out.normalIdx   = static_cast<int32_t>(material.normalTexture->textureIndex);
        out.normalScale = material.normalTexture->scale;
    }

    if (material.pbrData.metallicRoughnessTexture.has_value()) {
        out.metallicRoughnessIdx = static_cast<int32_t>(material.pbrData.metallicRoughnessTexture->textureIndex);
    }

    if (material.emissiveTexture.has_value()) {
        out.emissiveIdx = static_cast<int32_t>(material.emissiveTexture->textureIndex);
    }

    if (material.occlusionTexture.has_value()) {
        out.occlusionIdx      = static_cast<int32_t>(material.occlusionTexture->textureIndex);
        out.occlusionStrength = material.occlusionTexture->strength;
    }

    //Factores PBR
    out.baseColorFactor = {
        material.pbrData.baseColorFactor.x(),
        material.pbrData.baseColorFactor.y(),
        material.pbrData.baseColorFactor.z(),
        material.pbrData.baseColorFactor.w()
    };

    out.metallicFactor    = material.pbrData.metallicFactor;
    out.roughnessFactor   = material.pbrData.roughnessFactor;
    out.emissiveFactor    = {
        material.emissiveFactor.x(),
        material.emissiveFactor.y(),
        material.emissiveFactor.z()
    };
    out.emissiveStrength  = material.emissiveStrength;
    out.doubleSided       = material.doubleSided;
    out.alphaMode         = ConvertAlphaMode(material.alphaMode);
    out.alphaCutoff       = material.alphaCutoff;

    return out;
}



AlphaMode GltfImporter::ConvertAlphaMode(fastgltf::AlphaMode mode){
    switch (mode) {
        case fastgltf::AlphaMode::Blend:    return  AlphaMode::Blend;
        case fastgltf::AlphaMode::Mask :    return  AlphaMode::Mask;
        case fastgltf::AlphaMode::Opaque:   return  AlphaMode::Opaque; 
        default:                            return  AlphaMode::Opaque;
    }

}


std::vector<Texture> GltfImporter::loadTextures(fastgltf::Asset& asset,const FileContext& context){
    std::vector<Texture> textures;
    for(auto& t : asset.textures){
        textures.push_back(loadTexture(t, asset,context));
    }

    return textures;
}


Texture GltfImporter::loadTexture(
    const fastgltf::Texture& texture,
    const fastgltf::Asset& asset,
    const FileContext& context)
{
    Texture out;

    // 1. Obtener el índice de la imagen
    std::optional<std::size_t> imageIndex = texture.imageIndex;

    // Si usas extensiones (basisu, webp, etc.) puedes añadirlas aquí:
    if (!imageIndex) imageIndex = texture.basisuImageIndex;
    if (!imageIndex) imageIndex = texture.webpImageIndex;
    if (!imageIndex) imageIndex = texture.ddsImageIndex;
    // etc.

    if (!imageIndex) {
        // No hay imagen asociada
        return out;
    }

    const fastgltf::Image& image = asset.images[*imageIndex];

    // Intentar ruta
    if (auto path = getImagePath(image, context.dirPath)) {
        out.filePath = path->string();
        loadKtx2(out, out.filePath);          // desde archivo
        return out;
    }

    // Intentar bytes
    if (auto bytes = getImageBytes(image, asset)) {
        loadKtx2(out, "", *bytes);            // desde memoria
        return out;
    }

    // Fallback
    loadKtx2(out, "assets/textures/common/builtin_white.ktx2");
    return out;
}


// Helpers para la MIERDA DE EXTRAER IMAGENES, CTM COMO LO ODIO

// Helper 1: sacar la ruta de una imagen (si es URI)

std::optional<std::filesystem::path> GltfImporter::getImagePath(
    const fastgltf::Image& image,
    const std::filesystem::path& gltfDirectory)
{
    if (auto* uri = std::get_if<fastgltf::sources::URI>(&image.data)) {
        // uri->uri es un fastgltf::URI
        std::filesystem::path relative = uri->uri.path();   // o uri->uri.fspath() según versión
        return gltfDirectory / relative;
    }
    return std::nullopt;
}


// Helper 2: sacar los bytes de una imagen (si están disponibles)

std::optional<std::span<const std::byte>> GltfImporter::getImageBytes(
    const fastgltf::Image& image,
    const fastgltf::Asset& asset)
{
    // Caso 1: ya están en memoria (Array)
    if (auto* arr = std::get_if<fastgltf::sources::Array>(&image.data)) {
        return std::span<const std::byte>(arr->bytes.data(), arr->bytes.size());
    }

    // Caso 2: Vector
    if (auto* vec = std::get_if<fastgltf::sources::Vector>(&image.data)) {
        return std::span<const std::byte>(vec->bytes.data(), vec->bytes.size());
    }

    // Caso 3: ByteView
    if (auto* view = std::get_if<fastgltf::sources::ByteView>(&image.data)) {
        return view->bytes;
    }

    // Caso 4: BufferView (embebido en el GLB)
    if (auto* bv = std::get_if<fastgltf::sources::BufferView>(&image.data)) {
        const auto& bufferView = asset.bufferViews[bv->bufferViewIndex];
        const auto& buffer     = asset.buffers[bufferView.bufferIndex];

        // Aquí necesitas que el buffer ya tenga los bytes cargados
        if (auto* arr = std::get_if<fastgltf::sources::Array>(&buffer.data)) {
            const std::byte* start = arr->bytes.data() + bufferView.byteOffset;
            return std::span<const std::byte>(start, bufferView.byteLength);
        }
        if (auto* vec = std::get_if<fastgltf::sources::Vector>(&buffer.data)) {
            const std::byte* start = vec->bytes.data() + bufferView.byteOffset;
            return std::span<const std::byte>(start, bufferView.byteLength);
        }
    }

    return std::nullopt;
}

bool GltfImporter::loadKtx2(Texture& out, 
              const std::string& filePath,                    // puede estar vacío
              std::span<const std::byte> bytes)          // puede estar vacío
{
    KTX_error_code result = KTX_SUCCESS;

    if (!filePath.empty()) {
        // Caso 1: archivo externo 
        result = ktxTexture2_CreateFromNamedFile(
            filePath.c_str(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &out.ktxTex);
    }
    else if (!bytes.empty()) {
        // Caso 2: datos en memoria 
        result = ktxTexture2_CreateFromMemory(
            reinterpret_cast<const ktx_uint8_t*>(bytes.data()),
            static_cast<ktx_size_t>(bytes.size()),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &out.ktxTex);
    }
    else {
        // Ni ruta ni bytes → fallback
        result = KTX_INVALID_VALUE;
    }

    if (result != KTX_SUCCESS) {
        spdlog::warn("No se pudo cargar textura, usando fallback blanco");
        result = ktxTexture2_CreateFromNamedFile(
            "assets/textures/common/builtin_white.ktx2",
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &out.ktxTex);

        if (result != KTX_SUCCESS) return false;
    }

    // Transcoding 
    if (ktxTexture2_NeedsTranscoding(out.ktxTex)) {
        result = ktxTexture2_TranscodeBasis(out.ktxTex, KTX_TTF_BC7_RGBA, 0);
        if (result != KTX_SUCCESS) {
            spdlog::error("Fallo al transcodificar: {}", ktxErrorString(result));
            ktxTexture_Destroy(ktxTexture(out.ktxTex));
            out.ktxTex = nullptr;
            return false;
        }
    }

  
    out.width  = out.ktxTex->baseWidth;
    out.height = out.ktxTex->baseHeight;
    out.mips   = out.ktxTex->numLevels;

    return true;
}

std::vector<Mesh> GltfImporter::loadMeshes(fastgltf::Asset& asset){
    std::vector<Mesh> meshes;
    for(auto& m : asset.meshes){
        meshes.emplace_back(loadMesh(m,asset));
    }
    return meshes;
}

Mesh GltfImporter::loadMesh(const fastgltf::Mesh& mesh,fastgltf::Asset& asset){
    Mesh out;
    

    for (const auto& prim : mesh.primitives) {
        auto posIt = prim.findAttribute("POSITION");
        if (posIt == prim.attributes.end()) continue;

        const auto& posAccessor = asset.accessors[posIt->accessorIndex];
        uint32_t vertexCount = static_cast<uint32_t>(posAccessor.count);

        // Configurar offsets de la submesh
        Submesh submesh{};
        submesh.baseVertex = static_cast<int32_t>(out.vertices.size());
        submesh.firstIndex = static_cast<uint32_t>(out.indices.size());
        submesh.materialIndex = static_cast<uint32_t>(prim.materialIndex.value_or(0));

        // Expandir el buffer de vértices y cargar atributos
        std::size_t baseVertexOffset = out.vertices.size();
        out.vertices.resize(baseVertexOffset + vertexCount);

        loadAttribute<glm::vec3>(asset, prim, "POSITION",   out.vertices, &Vertex::position, baseVertexOffset);
        loadAttribute<glm::vec3>(asset, prim, "NORMAL",     out.vertices, &Vertex::normal,   baseVertexOffset);
        loadAttribute<glm::vec2>(asset, prim, "TEXCOORD_0", out.vertices, &Vertex::uv0,       baseVertexOffset);
        loadAttribute<glm::vec4>(asset, prim, "TANGENT", out.vertices,&Vertex::tangent,   baseVertexOffset);

        // Cargar Índices
        if (prim.indicesAccessor.has_value()) {
            const auto& indexAccessor = asset.accessors[prim.indicesAccessor.value()];
            submesh.indexCount = static_cast<uint32_t>(indexAccessor.count);

            std::size_t baseIndexOffset = out.indices.size();
            out.indices.resize(baseIndexOffset + submesh.indexCount);

            // Llamada a loadIndices escribiendo directo en el std::vector
            loadIndices(asset, prim, out.indices.data() + baseIndexOffset);
        } else {
            // Fallback para dibujado no indexado
            submesh.indexCount = vertexCount;
            for (uint32_t i = 0; i < vertexCount; ++i) {
                out.indices.push_back(i);
            }
        }

        out.submeshes.push_back(submesh);
    }

    computeTangentsIfMissing(out);
    return out;
}



void GltfImporter::computeTangentsIfMissing(Mesh& mesh) {
    if (mesh.vertices.empty() || mesh.indices.size() < 3)
        return;

    // Si ya hay alguna tangente distinta de cero, asumimos que vinieron del archivo
    bool hasTangents = std::any_of(mesh.vertices.begin(), mesh.vertices.end(),
        [](const Vertex& v) {
            return v.tangent != glm::vec4(0.0f);
        });
    if (hasTangents)
        return;

    std::vector<glm::vec3> tan1(mesh.vertices.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> tan2(mesh.vertices.size(), glm::vec3(0.0f));

    // Cada submesh tiene sus propios índices (locales) + baseVertex
    for (const Submesh& sub : mesh.submeshes) {
        if (sub.indexCount < 3)
            continue;

        const uint32_t indexEnd = sub.firstIndex + sub.indexCount;

        for (uint32_t i = sub.firstIndex; i + 2 < indexEnd; i += 3) {
            // Índices locales del primitivo → índices globales en mesh.vertices
            const uint32_t i0 = static_cast<uint32_t>(sub.baseVertex) + mesh.indices[i];
            const uint32_t i1 = static_cast<uint32_t>(sub.baseVertex) + mesh.indices[i + 1];
            const uint32_t i2 = static_cast<uint32_t>(sub.baseVertex) + mesh.indices[i + 2];

            if (i0 >= mesh.vertices.size() || i1 >= mesh.vertices.size() || i2 >= mesh.vertices.size())
                continue;

            const glm::vec3& p0 = mesh.vertices[i0].position;
            const glm::vec3& p1 = mesh.vertices[i1].position;
            const glm::vec3& p2 = mesh.vertices[i2].position;

            const glm::vec2& uv0 = mesh.vertices[i0].uv0;
            const glm::vec2& uv1 = mesh.vertices[i1].uv0;
            const glm::vec2& uv2 = mesh.vertices[i2].uv0;

            const glm::vec3 edge1 = p1 - p0;
            const glm::vec3 edge2 = p2 - p0;
            const glm::vec2 deltaUV1 = uv1 - uv0;
            const glm::vec2 deltaUV2 = uv2 - uv0;

            const float det = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
            if (std::abs(det) < 1e-8f)
                continue; // UVs degeneradas

            const float r = 1.0f / det;
            const glm::vec3 tangent   = (edge1 * deltaUV2.y - edge2 * deltaUV1.y) * r;
            const glm::vec3 bitangent = (edge2 * deltaUV1.x - edge1 * deltaUV2.x) * r;

            tan1[i0] += tangent;  tan1[i1] += tangent;  tan1[i2] += tangent;
            tan2[i0] += bitangent; tan2[i1] += bitangent; tan2[i2] += bitangent;
        }
    }

    // Ortogonalizar y guardar tangente + signo de bitangente (w)
    for (size_t i = 0; i < mesh.vertices.size(); ++i) {
        const glm::vec3& n = mesh.vertices[i].normal;
        const glm::vec3& t = tan1[i];

        // Si no se acumuló nada (vértice no usado o UVs malas), dejar algo usable
        if (glm::dot(t, t) < 1e-12f) {
            mesh.vertices[i].tangent = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
            continue;
        }

        // Gram-Schmidt contra la normal
        glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));

        // Signo de la bitangente para reconstruirla en el shader
        const float w = (glm::dot(glm::cross(n, t), tan2[i]) < 0.0f) ? -1.0f : 1.0f;

        mesh.vertices[i].tangent = glm::vec4(tangent, w);
    }
}