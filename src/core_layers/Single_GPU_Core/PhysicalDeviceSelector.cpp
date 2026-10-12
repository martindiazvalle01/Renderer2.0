#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "PhysicalDeviceSelector.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include <map>

vk::raii::PhysicalDevice PhysicalDeviceSelector::pick(const Instance& instance, const Surface& surface) {
     auto devices = instance.get().enumeratePhysicalDevices();
    if (devices.empty()) {
        throw std::runtime_error("No hay GPUs que soporte Vulkan.");
    }

    std::multimap<uint32_t, vk::raii::PhysicalDevice> candidates;
    for (auto const& device : devices) {
        uint32_t score = scoreDevice(device,surface);
        candidates.insert({score, device});
    }

    if (candidates.rbegin()->first == 0) {
        throw std::runtime_error("Ninguna GPU cumple con los requisitos mínimos.");
    }

    //Ya que es un multimap, no seleccionamos el primero porque esa es la llave (el puntaje obtenido), el segundo es el dispositivo fisico
    return candidates.rbegin()->second; // mayor puntaje
}



//Una abominacion de codigo que cree juntando lo que pedia how to vulkan y vkguide
uint32_t PhysicalDeviceSelector::scoreDevice(vk::raii::PhysicalDevice const& physicalDevice,const Surface& surface)
{
    vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();

    uint32_t score = 0;

    if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
        score += 1000;
    } else if (properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
        score += 100;
    }

    score += properties.limits.maxImageDimension2D;

    // --- Requisitos de vkguide / How to Vulkan ---

    // 1. Swapchain
    auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
    bool hasSwapchain = std::any_of(extensions.begin(), extensions.end(),
        [](auto const& ext) {
            return std::strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
        });
    if (!hasSwapchain) {
        return 0;
    }

    // 2. Queue de gráficos + present
    auto queueProps = physicalDevice.getQueueFamilyProperties();
    bool hasGraphicsAndPresent = false;
    for (uint32_t i = 0; i < queueProps.size(); ++i) {
        bool graphics = static_cast<bool>(queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics);
        bool present  = physicalDevice.getSurfaceSupportKHR(i, *surface.get());
        if (graphics && present) {
            hasGraphicsAndPresent = true;
            break;
        }
    }
    if (!hasGraphicsAndPresent) {
        return 0;
    }

    // 3. Features requeridas por vkguide
    auto features = physicalDevice.getFeatures2<
        vk::PhysicalDeviceFeatures2,
        vk::PhysicalDeviceVulkan12Features,
        vk::PhysicalDeviceVulkan13Features,
        vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
    >();

    auto const& feat2 = features.get<vk::PhysicalDeviceFeatures2>().features;
    auto const& v12   = features.get<vk::PhysicalDeviceVulkan12Features>();
    auto const& v13   = features.get<vk::PhysicalDeviceVulkan13Features>();
    auto const& ext   = features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

    if (!feat2.samplerAnisotropy)          return 0;
    if (!feat2.sampleRateShading)          return 0;
    if (!v12.bufferDeviceAddress)          return 0;
    if (!v12.descriptorIndexing)           return 0;
    if (!v12.shaderSampledImageArrayNonUniformIndexing) return 0;
    if (!v12.descriptorBindingVariableDescriptorCount)    return 0;
    if (!v12.descriptorBindingPartiallyBound)             return 0;
    if (!v12.runtimeDescriptorArray)       return 0;
    if (!v13.dynamicRendering)             return 0;
    if (!v13.synchronization2)             return 0;
    if (!ext.extendedDynamicState)         return 0;

    return score;
}
 
