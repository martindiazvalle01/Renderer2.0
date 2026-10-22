#include <cstdint>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <spdlog/spdlog.h>
#include "Swapchain.hpp"
#include "vulkan/vulkan.hpp"


// Crea un Struct con todas las capacidades de la superficie
void Swapchain::querySurfaceCapabilities(const vk::raii::PhysicalDevice& physicalDevice,const vk::raii::SurfaceKHR& surface){
    this->surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);  
}
//Escoje la cantidad de imagenes que tendra la swapchain (actualmente usa 3 , 2 minimo +1 para triple buffering)
void Swapchain::chooseImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities){
    uint32_t minImageCount = surfaceCapabilities.minImageCount;
    uint32_t maxImageCount = surfaceCapabilities.maxImageCount;
    this->imageCount = minImageCount + 1;
    if (maxImageCount > 0 && imageCount > maxImageCount) {
        this->imageCount = maxImageCount;
    }
}

vk::Extent2D Swapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,const vk::Extent2D& requestedExtent){
    //Si no existe un extent actual retorna el que usa actualmente la superficie
    if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
        return surfaceCapabilities.currentExtent;
    }
    //Esto es para la Re creacion del swapchain (cuando se cambie de tamaño la ventana de la aplicacion)
    return vk::Extent2D{
        std::clamp(requestedExtent.width,  surfaceCapabilities.minImageExtent.width,  surfaceCapabilities.maxImageExtent.width),
        std::clamp(requestedExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height)
    };
}

//Selecciona el formato de la superficie, por ahora solo usa B8G8R8A8Srgb y de colorSpace usa RGB no lineal
vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
    if (availableFormats.empty()) {
        throw std::runtime_error("No se encontraron formatos de superficie disponibles");
    }
    for (const auto& surfaceFormat : availableFormats) {
        if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
            return surfaceFormat;
        }
    }
    
    return availableFormats[0];
}

//Selecciona el modo de Presentacion, Por ahora simplemente selecciona el eMailbox y si no lo encuentra el FIFO
//TODO: Hacer que el usuario Pueda seleccionar el modo desde el layer de aplicacion
vk::PresentModeKHR Swapchain::choosePresentationMode(const std::vector<vk::PresentModeKHR>& availableModes) {
    for (const auto& availablePresentMode : availableModes) {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
            return availablePresentMode;
        }
    }
    return vk::PresentModeKHR::eFifo;
}

void Swapchain::selectFormat(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface) {
    std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
    surfaceFormat = chooseSurfaceFormat(availableFormats);
}

void Swapchain::selectPresentMode(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface) {
    std::vector<vk::PresentModeKHR> availableModes = physicalDevice.getSurfacePresentModesKHR(surface);
    presentationMode = choosePresentationMode(availableModes);
}

void Swapchain::createSwapchain(const vk::raii::SurfaceKHR& surface,
                                const vk::raii::Device& device,uint32_t graphicsIndex,
                                 uint32_t presentIndex,
                                 vk::SwapchainKHR oldSwapchain = nullptr) {
  
 
    vk::SwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.surface          = surface;
    swapchainCreateInfo.minImageCount    = imageCount;
    swapchainCreateInfo.imageFormat      = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent      = extent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

    std::array<uint32_t, 2> queueFamilyIndices = { graphicsIndex, presentIndex };

    //Si el index de la familia de queue de graficos y present es diferente activa el sharing mode en forma concurrente
    if (graphicsIndex != presentIndex) {
        swapchainCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
        swapchainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices.data();
    } else {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
        
    }

    swapchainCreateInfo.preTransform     = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapchainCreateInfo.presentMode      = presentationMode;
    swapchainCreateInfo.clipped          = VK_TRUE;
    swapchainCreateInfo.oldSwapchain     = oldSwapchain;

 
    this->swapchain = vk::raii::SwapchainKHR(device, swapchainCreateInfo);
 
}
 
void Swapchain::createImageViews(const vk::raii::Device& device) {
    swapchainImages = swapchain.getImages();
    imageViews.clear();
    imageViews.reserve(swapchainImages.size());

    for (uint32_t i = 0; i < swapchainImages.size(); i++) {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image                           = swapchainImages[i];
        viewInfo.viewType                        = vk::ImageViewType::e2D;
        viewInfo.format                          = surfaceFormat.format;
        viewInfo.components.r                    = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.g                    = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.b                    = vk::ComponentSwizzle::eIdentity;
        viewInfo.components.a                    = vk::ComponentSwizzle::eIdentity;
        viewInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel   = 0;
        viewInfo.subresourceRange.levelCount     = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount     = 1;

        imageViews.emplace_back(device, viewInfo);
    }
}

//Re hace la swapchain si se hace algun cambio a esta, normalmente se usa casi siempre cuando se cambie la resolucion de pantalla
void Swapchain::recreate(const vk::raii::Device& device,
                          const vk::raii::PhysicalDevice& physicalDevice,
                          const vk::raii::SurfaceKHR& surface,
                          uint32_t graphicsIndex,
                          uint32_t presentIndex,
                          vk::Extent2D requestedExtent){
    
    device.waitIdle();
    querySurfaceCapabilities(physicalDevice, surface);
    extent = chooseExtent(surfaceCapabilities, requestedExtent);
    vk::SwapchainKHR oldSwapchain = *swapchain;
    createSwapchain(surface, device, graphicsIndex, presentIndex,oldSwapchain);
    createImageViews(device);

}


Swapchain::Swapchain(const vk::raii::SurfaceKHR& surface, const vk::raii::PhysicalDevice& physicalDevice,
                    const vk::raii::Device& device, vk::Extent2D requestedExtent,
                    uint32_t graphicsIndex, uint32_t presentIndex) {
    querySurfaceCapabilities(physicalDevice, surface);
    chooseImageCount(surfaceCapabilities);
    extent = chooseExtent(surfaceCapabilities, requestedExtent);
    selectFormat(physicalDevice, surface);
    selectPresentMode(physicalDevice, surface);
    createSwapchain(surface, device, graphicsIndex, presentIndex);
    createImageViews(device);
}