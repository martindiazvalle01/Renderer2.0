#pragma once
#include "AssetPool.hpp"
#include <structTypes.hpp>
#include <unordered_map>
class AssetManager{

    public:
        Handle<Model> loadModel(IntermediateStruct&& data);
        void unloadModel(Handle<Model> model){
            modelPool.Release(model);
        }

         
        AssetPool<Model>&          GetModelPool()          { return modelPool; }
        const AssetPool<Model>&    GetModelPool()    const { return modelPool; }

        AssetPool<Mesh>&           GetMeshPool()           { return meshPool; }
        const AssetPool<Mesh>&     GetMeshPool()     const { return meshPool; }

        AssetPool<Material>&       GetMaterialPool()       { return materialPool; }
        const AssetPool<Material>& GetMaterialPool() const { return materialPool; }

        AssetPool<Texture>&        GetTexturePool()        { return texturePool; }
        const AssetPool<Texture>&  GetTexturePool()  const { return texturePool; }

    private:
        AssetPool<Model>    modelPool;
        AssetPool<Mesh>     meshPool;
        AssetPool<Material> materialPool;
        AssetPool<Texture>  texturePool;

        std::unordered_map<std::string, Handle<Texture>>  textureCache;
        Handle<Texture> GetOrCreateTexture(Texture&& data);
        Handle<Material>   loadMaterial(const MaterialData& materialData,std::vector<Handle<Texture>> textures);
        Handle<Mesh>     loadMesh(Mesh meshData);

};