

#include <spdlog/spdlog.h>
#include "ResourceManager.hpp"
#include "GltfSceneCache.hpp"
#include "MeshData.hpp"
#include "MaterialData.hpp"
#include "ResourceManagementTests.hpp"

void TestResourceLoading() {
    GltfSceneCache sceneCache;
    ResourceManager resourceManager;

    std::string gltfPath = "assets/models/props/DamagedHelmet/DamagedHelmet.gltf"; // el más chico que tengas, para probar rápido
    std::string basePath = std::filesystem::path(gltfPath).parent_path().string();

    // 1. Confirmar que el asset se parsea
    auto asset = sceneCache.get(gltfPath);
    if (!asset) {
        spdlog::error("TEST FALLÓ: no se pudo parsear {}", gltfPath);
        return;
    }
    spdlog::info("Asset parseado OK: {} meshes, {} materiales", asset->meshes.size(), asset->materials.size());

    // 2. Cargar el primer mesh y validar contenido
    auto mesh = resourceManager.load<MeshData>("test_mesh_0", gltfPath, 0, sceneCache, resourceManager, basePath);
    if (!mesh) {
        spdlog::error("TEST FALLÓ: MeshData no cargó");
        return;
    }
    spdlog::info("Mesh cargado: {} vértices, {} índices, {} submeshes",
                 mesh->vertices().size(), mesh->indices().size(), mesh->submeshes().size());

    // 3. Validar cada submesh y su material
    for (size_t i = 0; i < mesh->submeshes().size(); ++i) {
        const auto& sub = mesh->submeshes()[i];
        spdlog::info("  Submesh {}: offset={} count={} material={}",
                     i, sub.indexOffset, sub.indexCount, sub.material ? "OK" : "NULLPTR");

        if (sub.material) {
            spdlog::info("    baseColorFactor=({},{},{},{}) metallic={} roughness={}",
                sub.material->baseColorFactor.r, sub.material->baseColorFactor.g,
                sub.material->baseColorFactor.b, sub.material->baseColorFactor.a,
                sub.material->metallicFactor, sub.material->roughnessFactor);
            spdlog::info("    emissiveFactor=({},{},{}) alphaMode={} alphaCutoff={}",
                sub.material->emissiveFactor.r, sub.material->emissiveFactor.g, sub.material->emissiveFactor.b,
                static_cast<int>(sub.material->alphaMode), sub.material->alphaCutoff);

        // Antes solo teníamos este:
        spdlog::info("    baseColorTexture={}", sub.material->baseColorTexture ? "presente" : "nullptr");

        // Agregar los 4 que faltaban:
        spdlog::info("    metallicRoughnessTexture={}", sub.material->metallicRoughnessTexture ? "presente" : "nullptr");
        spdlog::info("    normalTexture={}", sub.material->normalTexture ? "presente" : "nullptr");
        spdlog::info("    occlusionTexture={}", sub.material->occlusionTexture ? "presente" : "nullptr");
        spdlog::info("    emissiveTexture={}", sub.material->emissiveTexture ? "presente" : "nullptr");

        // Bonus: si querés confirmar que la textura no solo "existe" sino que cargó bien de verdad
        if (sub.material->baseColorTexture) {
            spdlog::info("      baseColor: {}x{}, format={}", 
                    sub.material->baseColorTexture->width(), 
                    sub.material->baseColorTexture->height(),
                    static_cast<int>(sub.material->baseColorTexture->format()));
    }
}
        
    }
    if (!mesh->vertices().empty()) {
    const auto& v0 = mesh->vertices()[0];
    spdlog::info("  Vertex[0]: pos=({},{},{}) normal=({},{},{}) uv=({},{}) tangent=({},{},{},{})",
                 v0.position.x, v0.position.y, v0.position.z,
                 v0.normal.x, v0.normal.y, v0.normal.z,
                 v0.uv.x, v0.uv.y,
                 v0.tangent.x, v0.tangent.y, v0.tangent.z, v0.tangent.w);
}

    // 4. Probar el cache: cargar el mismo mesh de nuevo, debería devolver el mismo puntero
    auto meshAgain = resourceManager.load<MeshData>("test_mesh_0", gltfPath, 0, sceneCache, resourceManager, basePath);
    if (meshAgain.get() != mesh.get()) {
        spdlog::error("TEST FALLÓ: el cache no está devolviendo la misma instancia");
    } else {
        spdlog::info("Cache OK: misma instancia devuelta en la segunda carga");
    }
    
}