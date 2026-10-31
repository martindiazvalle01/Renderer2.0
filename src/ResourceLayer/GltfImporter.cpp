
#include "GltfImporter.hpp"
#include "fastgltf/types.hpp"
#include <structTypes.hpp>
#include <vector>
#include <glm/gtc/type_ptr.hpp>



std::vector<MaterialData>  GltfImporter::loadMaterials(fastgltf::Asset& asset,const FileContext& context){
    std::vector<MaterialData> materials;

    for(auto& m : asset.materials){
        materials.push_back(loadMaterial(asset,context,m));
    }
    
    return materials;


}

MaterialData GltfImporter::loadMaterial(fastgltf::Asset& asset,const FileContext& context, fastgltf::Material& material){
    MaterialData out;
    //Indices de texturas
    out.textureIdx = material.pbrData.baseColorTexture->textureIndex;
    out.normalIdx  = material.normalTexture->textureIndex;
    out.metallicRoughnessIdx = material.pbrData.metallicRoughnessTexture->textureIndex;
    out.emissiveIdx          = material.emissiveTexture->textureIndex;
    out.occlusionIdx         = material.occlusionTexture->textureIndex;

    // Variables PBR
    out.baseColorFactor      = {material.pbrData.baseColorFactor.x(),material.pbrData.baseColorFactor.y(),material.pbrData.baseColorFactor.z(),material.pbrData.baseColorFactor.w()};
    out.metallicFactor       = material.pbrData.metallicFactor;
    out.roughnessFactor      = material.pbrData.roughnessFactor;
    return out;

}