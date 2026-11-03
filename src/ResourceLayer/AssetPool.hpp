#pragma once

#include <ResourceLayer/structTypes.hpp>
#include <cstdint>
#include <vector>


template<typename T>
class AssetPool{
    public:
        struct Slot {
            T       Data;
            uint32_t generation = 1;
            uint32_t refCount   = 0;
            bool     alive      = false;
        };
        // Muy complicado colocar la funcion template en el .cpp, me da flojera
        Handle<T> Create(T&& asset){
            uint32_t index;
            // Si la lista no esta vacia, sacamos el ultimo indice libre de freelist
            if(!freeList.empty()) {
                index = freeList.back();
                freeList.pop_back();
            // si esta vacia, agregamos un indice a la lista
            }else {
                index = slots.size();
                slots.emplace_back();
            }

            slots[index].Data = std::move(asset);
            slots[index].alive = true;
            slots[index].refCount = 1;
            //Generation se mantiene o incrementa, tengo que revisar eso a detalle

            return {index, slots[index].generation};

        }
        
        //Funcion para agarrar un slot
        Slot* GetSlot(Handle<T> handle){
            // Si el index es mayor o igual al tamaño de slots nos devolvemos sin más
            if(handle.index >= slots.size()) return nullptr;

            Slot& slot = slots[handle.index];
            // revisamos que el handle sea valido (que no se haya destruido)
            if(!slot.alive || slot.generation != handle.generation) return nullptr;

            return &slot;
        }

        T* Get(Handle<T> handle){
           
            Slot* slot = GetSlot(handle);
            // Hacemos un chequeo de lo que retorna, porque hay que retornar el data del slot, y si es nullptr y hacemos un -> Data da error
            return slot ? &slot->Data : nullptr;
        }
        

        // Funcion para añadir una referencia
        void AddRef(Handle<T> handle){
            if(auto* slot = GetSlot(handle)){
                slot->refCount++;
            }
        }

        void Release(Handle<T> handle){
            if(auto* slot = GetSlot(handle)){
                slot-> refCount--;
                if(slot->refCount == 0){
                    //Destruimos el slot
                    slot->alive = false;
                    slot ->generation ++;   // Invalida todos los handles viejos
                    slot->Data = T{};       // libera memoria
                    freeList.push_back(handle.index);
                }
            }
        }

    
        std::vector<Slot> slots;
        std::vector<uint32_t> freeList; //Indices libres
};