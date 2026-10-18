#pragma once
#include "vulkan/vulkan.hpp"
#include <cstdint>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>


class Swapchain{

    public:
        Swapchain(const vk::raii::SurfaceKHR& surface,const vk::raii::PhysicalDevice& physicalDevice,const vk::raii::Device& device ,
                    vk::Extent2D requestedExtent,uint32_t graphicsIndex, uint32_t presentIndex);

        void recreate(const vk::raii::Device& device,
                          const vk::raii::PhysicalDevice& physicalDevice,
                          const vk::raii::SurfaceKHR& surface,
                          uint32_t graphicsIndex,
                          uint32_t presentIndex,
                          vk::Extent2D requestedExtent);
        
        const vk::raii::SwapchainKHR& get() const { return swapchain; }
        const std::vector<vk::raii::ImageView>& getImageViews() const { return imageViews; }
        const std::vector<vk::Image>& getImages() const { return swapchainImages; }
        vk::Format getFormat() const { return surfaceFormat.format; }
        vk::Extent2D getExtent() const { return extent; }
        uint32_t getImageCount() const { return imageCount; }
        vk::PresentModeKHR getPresentMode() const { return presentationMode; }

    private:

      
        vk::raii::SwapchainKHR swapchain = nullptr;
        
        std::vector<vk::Image> swapchainImages;
        
        //Resolucion de las imagenes de estaran dentro de la swapchain
        vk::Extent2D extent;
        uint32_t imageCount = 0;
        std::vector<vk::raii::ImageView> imageViews;

        //Formato en el Cual estara la Swapchain
        vk::SurfaceFormatKHR surfaceFormat;
        //El modo en el cual se van a enviar los framebuffers
        vk::PresentModeKHR presentationMode;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities{};

        void querySurfaceCapabilities(const vk::raii::PhysicalDevice& physicalDevice,const vk::raii::SurfaceKHR& surface);
        void chooseImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);
        vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities,const vk::Extent2D& extent);

        vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
        void selectFormat(const vk::raii::PhysicalDevice& physicalDevice,const vk::raii::SurfaceKHR& surface);
        void selectPresentMode(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface);
        vk::PresentModeKHR choosePresentationMode(const std::vector<vk::PresentModeKHR>& availableModes);

        void createSwapchain(const vk::raii::SurfaceKHR& surface, const vk::raii::Device& device,uint32_t graphicsIndex,
                                 uint32_t presentIndex,
                                 vk::SwapchainKHR oldSwapchain);
        void createImageViews(const vk::raii::Device& device);
        


};