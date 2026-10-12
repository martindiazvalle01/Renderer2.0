#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Instance.hpp"


class Surface {
public:
    void create(const Instance& instance, SDL_Window* window);
    const vk::raii::SurfaceKHR& get() const { return surface; }



private:
    vk::raii::SurfaceKHR surface = nullptr;
};