#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "Instance.hpp"
#include "Surface.hpp"
#include "PhysicalDeviceSelector.hpp"
#include "LogicalDevice.hpp"
#include "Allocator.hpp"
#include "DebugMessenger.hpp"
class RenderDevice {
public:
    void init(SDL_Window* window) {
        instance.create();
        debugMessenger.create(instance);
        surface.create(instance, window);
        physicalDevice = selector.pick(instance, surface);
        logicalDevice.create(physicalDevice,surface);
        allocator.create(instance, physicalDevice, logicalDevice);
        renderContext.emplace(RenderContext{
            instance.get(),
            logicalDevice.get(),
            physicalDevice,
            surface.get(),
            logicalDevice.getGraphicsQueue(),
            logicalDevice.getGraphicsIndex(),
            logicalDevice.getPresentIndex(),
            allocator.get()
        });

    }

    const Instance&       getInstance()       const { return instance; }
    const Surface&        getSurface()        const { return surface; }
    const vk::raii::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
    const LogicalDevice&  getLogicalDevice()  const { return logicalDevice; }
    const Allocator&      getAllocator()      const { return allocator; }
    
    struct RenderContext {
        const vk::raii::Instance&       instance;
        const vk::raii::Device&         device;
        const vk::raii::PhysicalDevice& physicalDevice;
        const vk::raii::SurfaceKHR&     surface;
        const vk::raii::Queue&          graphicsQueue;
        uint32_t graphicsIndex;
        uint32_t presentIndex;
        VmaAllocator allocator;
    };
    const RenderContext& context() const { return *renderContext; }

private:
    Instance instance;
    DebugMessenger debugMessenger;
    Surface surface;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    PhysicalDeviceSelector selector;
    LogicalDevice logicalDevice;
    Allocator allocator;
    std::optional<RenderContext> renderContext;

};