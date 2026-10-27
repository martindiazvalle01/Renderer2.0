
#pragma once
#include <MaterialData.hpp>
#include <unordered_map>
#include <memory>
#include <string>
#include <typeindex>
#include "Resource.hpp"
#include <memory>    // std::make_shared, std::shared_ptr
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

//TODO: Revisar su hacer el resource manager asincrono y resource streaming (en la parte de tecnicas avanzadas de vulkan)
struct MaterialData; 

class ResourceManagerGLTF {
public:
    // T es el tipo concreto (TextureData, MeshData...), pero se guarda
    // y se devuelve como Resource 
    template<typename T, typename... Args>
    std::shared_ptr<T> load(const std::string& id, Args&&... args) {
        auto& typeCache = cache[std::type_index(typeid(T))];
         // Busca si el resource ya esta cargado o no, y si esta cargado devuelve el mismo valor
        if (auto it = typeCache.find(id); it != typeCache.end()) {
            return std::static_pointer_cast<T>(it->second);
        }
        //Crea el recurso si no existe y lo carga, si load retorna falso la funcion retorna null
        auto resource = std::make_shared<T>(id, std::forward<Args>(args)...);
        if (!resource->Load()) {
            return nullptr;
        }
        //Se agrega el recurso con su ID a su tipo de cache 
        typeCache[id] = resource;
        return resource;
    }

    template<typename T>
    //Agarra el puntero a un recurso y lo retorna segun el ID
    std::shared_ptr<T> get(const std::string& id) {
        auto typeIt = cache.find(std::type_index(typeid(T)));
        if (typeIt == cache.end()) return nullptr;

        auto it = typeIt->second.find(id);
        return it != typeIt->second.end() ? it->second : nullptr;
    }

    template<typename T>
    //Funcion para borrar el archivo una vez su vida util termino
    void unload(const std::string& id) {
        auto typeIt = cache.find(std::type_index(typeid(T)));
        if (typeIt == cache.end()) return;

        auto it = typeIt->second.find(id);
        if (it != typeIt->second.end()) {
            it->second->Unload();
            typeIt->second.erase(it);
        }
    }

    // Recorre TODOS los recursos cacheados sin importar el tipo concreto
    template<typename Fn>
    void forEachResource(Fn&& fn) {
        for (auto& [type, typeCache] : cache) {
            for (auto& [id, resource] : typeCache) {
                fn(resource);
            }
        }
    }



    std::shared_ptr<MaterialData> loadMaterialFromGltf(
    const fastgltf::Asset& asset, size_t materialIndex, const std::string& basePath);


    

private:
    // type_index -> (id -> Resource). Esto reemplaza tener un mapa
    // separado por cada tipo (textures, meshes) 
    std::unordered_map<std::type_index,
                        std::unordered_map<std::string, std::shared_ptr<Resource>>> cache;
    
    std::unordered_map<std::string, std::shared_ptr<MaterialData>> materialCache;



    
};