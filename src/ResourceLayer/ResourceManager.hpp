#pragma once
#include "AssetManager.hpp"
#include <cstdio>
#include <unordered_map>
#include "structTypes.hpp"
#include "GltfImporter.hpp"

class ResourceManager{
    public:
        void load(const FileContext& context){
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
        

        

    private:
        std::unordered_map<std::string,Handle<Model>> models;
        AssetManager manager;
        GltfImporter gltfImporter;

};
