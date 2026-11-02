#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "spdlog/spdlog.h"
#include <ResourceLayer/structTypes.hpp>
#include <cstddef>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <fastgltf/tools.hpp>

class GltfImporter{
    public:
        // Esta es la funcion principal la unica que debe ser public 
        IntermediateStruct Import(const FileContext& context){
            fastgltf::Parser parser;
            auto data = fastgltf::GltfDataBuffer::FromPath(context.filePath);
            if(data.error() != fastgltf::Error::None) { 
                spdlog::error("Error al cargar GLTF desde '{}': {}", context.filePath.string(), fastgltf::getErrorMessage(data.error()));   
                throw std::runtime_error("Fallo al cargar el archivo de datos GLTF.");
            }

            auto result = parser.loadGltf(data.get(),context.filePath,fastgltf::Options::LoadExternalBuffers
                                | fastgltf::Options::LoadExternalImages);

            if(result.error() != fastgltf::Error::None){
                spdlog::error("Error al parsear GLTF en '{}': {}", context.filePath.string(), fastgltf::getErrorMessage(result.error()));
                throw std::runtime_error("Fallo crítico en el parser de fastgltf.");
            }

            fastgltf::Asset& asset = result.get();

            IntermediateStruct out;

            out.materials = loadMaterials(asset);
            out.meshes    = loadMeshes(asset);
            out.textures  = loadTextures(asset,context);

            return out;

        }
    private:

        std::vector<MaterialData>   loadMaterials(fastgltf::Asset& asset);
        std::vector<Texture>        loadTextures(fastgltf::Asset& asset,const FileContext& context);
        std::vector<Mesh>           loadMeshes(fastgltf::Asset& asset);

        MaterialData       loadMaterial(const fastgltf::Material& material);
        Texture        loadTexture(const fastgltf::Texture& texture, const fastgltf::Asset& asset, const FileContext& context);
        Mesh           loadMesh(const fastgltf::Mesh& mesh,fastgltf::Asset& asset);
        void computeTangentsIfMissing(Mesh& mesh);
        
        AlphaMode ConvertAlphaMode(fastgltf::AlphaMode mode);

        std::optional<std::filesystem::path> getImagePath(const fastgltf::Image& image,
                                                            const std::filesystem::path& gltfDirectory);
                            
        std::optional<std::span<const std::byte>> getImageBytes(
                                const fastgltf::Image& image,
                                const fastgltf::Asset& asset);
        
        bool loadKtx2(Texture& out, const std::string& filePath, std::span<const std::byte> bytes = {});
       

        //Funciones Helper para la extraccion de atributos (las cosas del mesh y submesh)
        template <typename T>
        void loadAttribute(const fastgltf::Asset& asset, 
                       const fastgltf::Primitive& prim, 
                       std::string_view name, 
                       std::vector<Vertex>& vertices, 
                       T Vertex::* memberPtr,std::size_t offset = 0) {

            auto it = prim.findAttribute(name);
            if (it == prim.attributes.end()) return;

            const auto& accessor = asset.accessors[it->accessorIndex];
            std::size_t i = offset;
            fastgltf::iterateAccessor<T>(asset, accessor, [&](const T& value) {vertices[i++].*memberPtr = value;});
        }

       // Helper modular: Copia los índices directamente al buffer destino
        uint32_t loadIndices(const fastgltf::Asset& asset, 
                         const fastgltf::Primitive& prim, 
                         uint32_t* destinationBuffer) 
        {
            if (!prim.indicesAccessor.has_value()) return 0;
            const auto& indexAccessor = asset.accessors[prim.indicesAccessor.value()];
    
            // fastgltf realiza la conversión automática de uint16_t/uint8_t a uint32_t
            fastgltf::copyFromAccessor<uint32_t>(asset, indexAccessor, destinationBuffer);
    
            return static_cast<uint32_t>(indexAccessor.count);
}
};