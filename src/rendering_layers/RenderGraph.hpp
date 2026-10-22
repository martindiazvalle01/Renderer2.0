#pragma once
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
            vk::ImageLayout currentLayout;

            // Recursos de GPU reales - rellenados durante la compilacion
            GpuImage image;    // clase que controla la imagen de en la GPU, esta contiene la localizacion de la asignacion de memoria
            vk::ImageView view = nullptr; // Vista de la imagen accesible para el shader
            vk::raii::ImageView importedView = nullptr;  // solo para recursos que crea Compile
            bool imported = false;  //  Si la imagen viene desde swapchain

            bool persistent = false; // si el recurso no es persistente, se reinician los layouts por cada frame

        };

        // Esta es la estructura la cual contiene la informacion de los render pass, se crearan varios passes como aparecen en los diagramas (Depth Pre-Pass, Shadow Pass)
        struct Pass {
            std::string name;                  // Nombre descriptivo para la clase para el debugging
            std::vector<std::string> inputs;   // Recursos de los que lee este pass (dependencias)
            std::vector<std::string> outputs;  // Recursos a los que escribe este pass (producto)
            std::function<void(vk::raii::CommandBuffer&)> executeFunc;  // Esto lo que hace es guardar el codigo que se necesitara ejecutar en el pass, no devuelve nada, 
                                                                    // esto representa el draw mesh en el diagrama, osea todo lo que es contenido especifico del pass
            
            //Esto es simplemente para indicar que passes estan encendidos y cuales no
            bool enabled = true;
        };

        //  Struct la cual contiene los flags de la etapa del pipeline y el tipo de acceso, se crearan 2 para el Src y el Dst
        struct BarrierHalf{
            vk::PipelineStageFlags stage;
            vk::AccessFlags access;
        };


        // Datos principales de almacenamiento para el sistema RenderGraph
        std::unordered_map<std::string, GraphResource> graphResources = {}; // Todos los recursos referenciados en el graph
        std::vector<Pass> passes;                                      // Todos los passes de renderizado ordenados 
        std::vector<size_t> executionOrder;                            // Secuencia de compute optima para ejecucion

        // Esto existe por si acaso me pasa de nuevo que se me olvida compilar antes de ejecutar
        bool compiled = false;

        //Estos objetos se aseguran de la correcta ejecucion de la GPU sin barreras manuales
        // TODO: Esto por ahora es codigo muerto, no se usa, para cuando el Renderer este funcional hay que crear estos para 
        //  que el los passes que tengan colas distintas se ejecuten en paralelo 
        std::vector<vk::raii::Semaphore> semaphores;
        std::vector<std::pair<size_t, size_t>> semaphoreSignalWaitPairs; //// (signaling pass, waiting pass)

        vk::ImageAspectFlags aspectFromFormat(vk::Format format);
        void TransitionResource(vk::raii::CommandBuffer& commandBuffer,
                        GraphResource& res, vk::ImageLayout newLayout);
        GraphResource& GetResourceChecked(const std::string& name);

        //Rellena el struct que tiene el layout del src
        BarrierHalf SrcHalf(vk::ImageLayout oldLayout);
        //Rellena el struct que tiene el layout del dst
        BarrierHalf DstHalf(vk::ImageLayout newLayout);

        //Funcion que decide el tipo de layout dependiendo del formato
        vk::ImageLayout LayoutForWrite(vk::Format format);
    public:
        void Compile(const vk::raii::Device& device,VmaAllocator allocator);

        void Execute(vk::raii::CommandBuffer& commandBuffer);
        void submitFrame( vk::raii::Queue& graphicsQueue);
        //Interfaz de acceso al recuso para recuperar recursos compilados
        GraphResource* GetResource(const std::string& name){
            auto it = graphResources.find(name);
            return(it != graphResources.end()) ? & it-> second : nullptr; 
        }

       

        void AddResource(const std::string& name, vk::Format format, vk::Extent2D extent,
                    vk::ImageUsageFlags usage, vk::ImageLayout initialLayout,
                    vk::ImageLayout finalLayout,bool imported = false);
        
        void AddPass(const std::string& name,
                const std::vector<std::string>& inputs,
                const std::vector<std::string>& outputs,
                std::function<void(vk::raii::CommandBuffer&)> executeFunc);

        void setImportedImage(const std::string& name, vk::Image image,  vk::ImageView imageView);

        void TransitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

        void RenderFrame(vk::raii::Device& device, vk::Queue graphicsQueue, vk::Queue presentQueue);




        
};