#include <Surface.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Surface.hpp"

void Surface::create(const Instance& instance,SDL_Window* window) {
    VkSurfaceKHR rawSurface;
 
    if (!SDL_Vulkan_CreateSurface(window, *instance.get(), nullptr, &rawSurface)) {
        throw std::runtime_error(std::string("Fallo al crear Surface: ") + SDL_GetError());
    }
    try {
        surface = vk::raii::SurfaceKHR(instance.get(), rawSurface);
    } catch (...) {
        // Si el RAII falla, hay que destruir rawSurface
        instance.get().getDispatcher()->vkDestroySurfaceKHR(*instance.get(), rawSurface, nullptr);
        throw;
    }
   
}