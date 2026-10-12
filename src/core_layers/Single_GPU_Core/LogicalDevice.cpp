#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

#include "LogicalDevice.hpp"
#include "Surface.hpp"

void LogicalDevice::create(const vk::raii::PhysicalDevice& physicalDevice, const Surface& surface)
{

    findQueueFamilies(physicalDevice, surface);

    float queuePriority = 1.0f;

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
        vk::DeviceQueueCreateFlags(),
        graphicsIndex,
        1,
        &queuePriority
    };

    //Features ordenadas por la version en la que fueron agregadas
    vk::StructureChain<vk::PhysicalDeviceFeatures2,
                       vk::PhysicalDeviceVulkan12Features,
                       vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain;

    //Features de Vulkan 1.2
    auto& v12 = featureChain.get<vk::PhysicalDeviceVulkan12Features>();
    //Estos 2 son para habilitar bindless texturing y para poder usar un puntero a un grupo de descriptor sets en lugar crear un set para cada textura entrante
    v12.setBufferDeviceAddress(VK_TRUE);
    v12.setDescriptorIndexing(VK_TRUE);

    // Permite que cada pixel/vertice acceda a un indice distinto dentro de un array de texturas
    v12.setShaderSampledImageArrayNonUniformIndexing(VK_TRUE);
    //Permite que el descriptor set tenga un tamaño variable sin desperdiciar memoria si el tamaño de un descriptor set es mayor al que realmente se usa
    v12.setDescriptorBindingVariableDescriptorCount(VK_TRUE);
    //Permite que algunos slots de un array de descriptor sets esten vacios sin que el shader falle
    v12.setDescriptorBindingPartiallyBound(VK_TRUE);
    //Permiten declarar arrays de descriptor sin especificar un tamaño fijo en tiempo de compilacion
    v12.setRuntimeDescriptorArray(VK_TRUE);

    v12.setTimelineSemaphore(VK_TRUE);

    auto& v13 = featureChain.get<vk::PhysicalDeviceVulkan13Features>();
    v13.setDynamicRendering(VK_TRUE);
    v13.setSynchronization2(VK_TRUE);

    auto& ext = featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
    ext.setExtendedDynamicState(VK_TRUE);

    auto& features2 = featureChain.get<vk::PhysicalDeviceFeatures2>();
    features2.features.setSampleRateShading(VK_TRUE);
    features2.features.setSamplerAnisotropy(VK_TRUE);

    std::vector<const char*> requiredDeviceExtension = {vk::KHRSwapchainExtensionName};

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.setPNext(&featureChain.get<vk::PhysicalDeviceFeatures2>());
    deviceCreateInfo.setQueueCreateInfoCount(1);
    deviceCreateInfo.setPQueueCreateInfos(&deviceQueueCreateInfo);
    deviceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(requiredDeviceExtension.size()));
    deviceCreateInfo.setPpEnabledExtensionNames(requiredDeviceExtension.data());

    device        = vk::raii::Device(physicalDevice, deviceCreateInfo);
    graphicsQueue = device.getQueue(graphicsIndex, 0);
}




void LogicalDevice::findQueueFamilies(const vk::raii::PhysicalDevice& physicalDevice, const Surface& surface) {
    auto props = physicalDevice.getQueueFamilyProperties();

    // Intento 1: buscar una sola familia que cumpla graphics + present a la vez
    for (uint32_t i = 0; i < props.size(); ++i) {
        bool graphics = static_cast<bool>(props[i].queueFlags & vk::QueueFlagBits::eGraphics);
        bool present  = physicalDevice.getSurfaceSupportKHR(i, *surface.get());
        if (graphics && present) {
            graphicsIndex = i;
            presentIndex  = i;
            return;
        }
    }

    // Si no encuentra una QueueFamily que soporte tanto graphics como present, los busca por separado
    for (uint32_t i = 0; i < props.size(); ++i) {
        if (graphicsIndex == UINT32_MAX && (props[i].queueFlags & vk::QueueFlagBits::eGraphics)) {
            graphicsIndex = i;
        }
        if (presentIndex == UINT32_MAX && physicalDevice.getSurfaceSupportKHR(i, *surface.get())) {
            presentIndex = i;
        }
    }

    if (graphicsIndex == UINT32_MAX) {
        throw std::runtime_error("No se encontró ninguna familia de colas con soporte de graphics");
    }
    if (presentIndex == UINT32_MAX) {
        throw std::runtime_error("No se encontró ninguna familia de colas con soporte de present");
    }
}