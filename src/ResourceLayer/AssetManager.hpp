#pragma once
#include "AssetPool.hpp"
#include <structTypes.hpp>
#include <unordered_map>
class AssetManager{

    public:
        Handle<Model> loadModel(const IntermediateStruct& data);
        void unloadModel(Handle<Model> model){
            modelPool.Release(model);
        }

         

    private:
        AssetPool<Model>    modelPool;
        AssetPool<Mesh>     meshPool;
        AssetPool<Material> materialPool;
        AssetPool<Texture>  texturePool;

        std::unordered_map<std::string, Handle<Texture>>  textureCache;
        Handle<Texture> GetOrCreateTexture(const Texture& data);
        Handle<Material>   loadMaterial(const MaterialData& materialData,std::vector<Handle<Texture>> textures);
        Handle<Mesh>     loadMesh(Mesh meshData);

};