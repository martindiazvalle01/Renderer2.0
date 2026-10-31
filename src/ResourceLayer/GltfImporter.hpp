#include "fastgltf/core.hpp"
#include "fastgltf/types.hpp"
#include "spdlog/spdlog.h"
#include <ResourceLayer/structTypes.hpp>
#include <spdlog/spdlog.h>
#include <vector>

class GltfImporter{
    public:
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

            out.materials = loadMaterials(asset, context);
            out.meshes    = loadMeshes(asset,context);
            out.textures  = loadTextures(asset,context);

            return out;

        }

        std::vector<MaterialData>   loadMaterials(fastgltf::Asset& asset,const FileContext& context);
        std::vector<Texture>        loadTextures(fastgltf::Asset& asset,const FileContext& context);
        std::vector<Mesh>           loadMeshes(fastgltf::Asset& asset,const FileContext& context);

        MaterialData       loadMaterial(fastgltf::Asset& asset,const FileContext& context, fastgltf::Material& material);
        Texture        loadTexture(fastgltf::Asset& asset,const FileContext& context);
        Mesh           loadMesh(fastgltf::Asset& asset,const FileContext& context);



};