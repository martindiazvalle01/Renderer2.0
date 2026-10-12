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
    }

    const Instance&       getInstance()       const { return instance; }
    const Surface&        getSurface()        const { return surface; }
    const vk::raii::PhysicalDevice& getPhysicalDevice() const { return physicalDevice; }
    const LogicalDevice&  getLogicalDevice()  const { return logicalDevice; }
    const Allocator&      getAllocator()      const { return allocator; }

private:
    Instance instance;
    DebugMessenger debugMessenger;
    Surface surface;
    vk::raii::PhysicalDevice physicalDevice = nullptr;
    PhysicalDeviceSelector selector;
    LogicalDevice logicalDevice;
    Allocator allocator;
};