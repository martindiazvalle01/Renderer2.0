#pragma once

#include "vulkan/vulkan.hpp"
#include <array>


#include <cstdint>
#include <vulkan/vulkan_raii.hpp>
//No se que numero colocar como maximo de imagenes, asi que simplemente colocare un numero alto
#define Max_Textures 4096

// Esta parte esta sacada de https://dev.to/gasim/implementing-bindless-design-in-vulkan-34no

// Esta clase administra el catálogo de texturas que ven los shaders.
// El descriptor set es un catalogo con N casilleros numerados, cada casillero
// apunta a una textura que ya está en la GPU (su view y su sampler).
// Aqui se crean el catalogo y se escribe en cada casillero la referencia
// a una textura. El shader usa el número del casillero para encontrarla.
// Los casilleros se llenan y se vacían a medida que las texturas se cargan y se descargan
// quién decide qué casillero usar lo maneja otra clase

class DescriptorManager{

    public:
        void init(const vk::raii::Device& device);
        // Esta clase guarda la textura en un  descriptorSet, y devuelve su posicion
        void storeTexture(uint32_t index,vk::ImageView imageView, vk::Sampler sampler);
        const vk::raii::DescriptorSetLayout& GetLayout() const { return bindlessLayout; }
        const vk::raii::DescriptorSet& GetDescriptorSet() const { return bindlessDescriptorSet; }
    private:

        const vk::raii::Device* device;

        std::array<vk::DescriptorSetLayoutBinding,1> bindings{};
        std::array<vk::DescriptorBindingFlags,1> flags{};
        std::array<vk::DescriptorType, 1> types{
            vk::DescriptorType::eCombinedImageSampler
        };

        vk::raii::DescriptorSetLayout bindlessLayout = nullptr;
         vk::raii::DescriptorPool pool = nullptr;
        vk::raii::DescriptorSet bindlessDescriptorSet = nullptr;
       
        
};