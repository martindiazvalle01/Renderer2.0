#pragma once
#include "AssetManager.hpp"
#include <AssetPool.hpp>
#include <cstdio>
#include <unordered_map>
#include "structTypes.hpp"
#include "GltfImporter.hpp"

class ResourceManager{
    public:
        void load(const FileContext& context){
            printf("cwd: %s\n", std::filesystem::current_path().c_str());
printf("existe: %d\n", std::filesystem::exists(context.filePath));
            AssetFormat formato = formatFromExtension(context.filePath);
            IntermediateStruct structIntermedio;

            switch (formato) {
                case AssetFormat::GLTF:
                    structIntermedio = gltfImporter.Import(context);
                    break;
                
                default:
                    printf("Formato no compatible");
                    return;
            }
            models[context.filePath] = manager.loadModel(structIntermedio);
        }

        

        void unload(const FileContext& context){
            Handle<Model> model = models[context.filePath];
            manager.unloadModel(model);
        }

        Model& getModel(const FileContext& context){
            Handle<Model> model = models[context.filePath];
            return *manager.GetModelPool().Get(model);
        }
        

        AssetManager&       GetAssetManager()       { return manager; }
        const AssetManager& GetAssetManager() const { return manager; }
        

    private:
        std::unordered_map<std::string,Handle<Model>> models;
        AssetManager manager;
        GltfImporter gltfImporter;

};
