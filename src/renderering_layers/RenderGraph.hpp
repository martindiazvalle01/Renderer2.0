
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

#include "GpuImage.hpp"



class RenderGraph{


    private:
        //  Descripcion del recurso y su estructura
        //  Representa el recurso de la imagen usada durante el renderizado (Texturas)
        struct GraphResource{
            std::string name;                   //  Nombre para identificarlo en el debugging    
            vk::Format format;                  //  Formato a nivel pixel (RGBA8, Depth24Stencil8,etc.)
            vk::Extent2D extent;                //  Dimensiones en pixeles del recurso 2D (Osea el tamaño de la textura)
            vk::ImageUsageFlags usage;          //  Como va a ser usado este recurso (color attachment,texture, etc.)
            vk::ImageLayout initialLayout;      //  Layout esperado cuando empieze el frame
            vk::ImageLayout finalLayout;        //  Layout requerido cuando el frame termine 
            vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;  // se actualiza cada vez que se inserta un barrier

            // Recursos de GPU reales - rellenados durante la compilacion
            GpuImage image;    // clase que controla la imagen de en la GPU, esta contiene la localizacion de la asignacion de memoria
            vk::raii::ImageView view = nullptr; // Vista de la imagen accesible para el shader
        };

        // Esta es la estructura la cual contiene la informacion de los render pass, se crearan varios passes como aparecen en los diagramas (Depth Pre-Pass, Shadow Pass)
        struct Pass {
            std::string name;                  // Nombre descriptivo para la clase para el debugging
            std::vector<std::string> inputs;   // Recursos de los que lee este pass (dependencias)
            std::vector<std::string> outputs;  // Recursos a los que escribe este pass (producto)
            std::function<void(vk::raii::CommandBuffer&)> executeFunc;  // Esto lo que hace es guardar el codigo que se necesitara ejecutar en el pass, no devuelve nada, 
                                                                    // esto representa el draw mesh en el diagrama, osea todo lo que es contenido especifico del pass
        };

        // Datos principales de almacenamiento para el sistema RenderGraph
        std::unordered_map<std::string, GraphResource> graphResources; // Todos los recursos referenciados en el graph
        std::vector<Pass> passes;                                      // Todos los passes de renderizado ordenados 
        std::vector<size_t> executionOrder;                            // Secuencia de compute optima para ejecucion


        //Estos objetos se aseguran de la correcta ejecucion de la GPU sin barreras manuales
        // TODO: Esto por ahora es codigo muerto, no se usa, para cuando el Renderer este funcional hay que crear estos para 
        //  que el los passes que tengan colas distintas se ejecuten en paralelo 
        std::vector<vk::raii::Semaphore> semaphores;
        std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs; //// (signaling pass, waiting pass)

        vk::ImageAspectFlags aspectFromFormat(vk::Format format);
    
    public:
        void compile(vk::raii::Device& device,VmaAllocator allocator);

        //Interfaz de acceso al recuso para recuperar recursos compilados
        GraphResource* GetResource(const std::string& name){
            auto it = graphResources.find(name);
            return(it != graphResources.end()) ? & it-> second : nullptr; 
        }

};