#pragma once
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>

#define MaxFramesInFlight 3

class FrameManager{
    private:
       
        struct FrameContext {
            vk::raii::CommandPool   pool{nullptr};
            std::vector<vk::raii::CommandBuffer> cmd; 
            vk::raii::Semaphore     imageAvailable{nullptr};  // Semaforo para cuando el sistema de presentacion (cosas del driver) termina de usar la imagen del swapchain, a lo que termine se puede empezar a dibujar
            vk::raii::Semaphore     renderFinished{nullptr}; //Semaforo para indicar cuando la GPU termina de dibujar la imagen para poder enviarla al sistema de presentacion.
            uint64_t                retireValue = 0; // el valor del timeline que hay que esperar para reusar este slot
        };

        std::array<FrameContext, MaxFramesInFlight> frames;
        
        //Un solo semaforo compartido por todos los frames para avisar cuando uno esta listo
        vk::raii::Semaphore timeline{nullptr}; 
        //Contador que va incrementando por cada frame nuevo
        uint64_t nextSubmitValue = 1;

        //FrameActual
        uint64_t frameCount = 0;

        vk::raii::CommandPool   createCommandPool(const vk::raii::Device& device, uint32_t queueFamilyIndex);
        //De default el nivel del command buffer es primary, pero pueden ingresar de forma secundary para cuando se agregue multi threading en el futuro
        std::vector<vk::raii::CommandBuffer> createCommandBuffer(const vk::raii::Device& device,const vk::CommandPool& pool,uint32_t count = 1,
                                                             vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary);
        
        void createTimelineSemaphore(const vk::raii::Device& device);
        void createFrameSemaphores(const vk::raii::Device& device, FrameContext& frame);
        void createContext(const vk::raii::Device& device, uint32_t queueFamilyIndex);

    public:
            FrameManager(const vk::raii::Device& device, uint32_t queueFamilyIndex);

            
            // Se usa optional porque si retorna el index de la imagen significa que el codigo se ejecuto correctamente
            // Si no lo hizo y retorna std::nullopt significa que hay que rehacer la swapchain
            std::optional<uint32_t> beginFrame(const vk::raii::Device& device,vk::raii::SwapchainKHR& swapchain);

            bool submitFrame(const vk::raii::Queue& graphicsQueue,
                                vk::raii::SwapchainKHR& swapchain, uint32_t imageIndex);



};