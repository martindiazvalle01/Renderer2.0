#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "Instance.hpp"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

void Instance::create() {

    //Numero de extensiones de sdl
    unsigned int sdlExtensionCount = 0;

    const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    if (sdlExtensions == nullptr) {
        throw std::runtime_error(std::string("SDL_Vulkan_GetInstanceExtensions falló: ") + SDL_GetError());
    }

    std::vector<const char*> extensionNames;
    extensionNames.reserve(sdlExtensionCount + 1);  // +1 para la extensión de debug

   for (Uint32 i = 0; i < sdlExtensionCount; ++i) {
        extensionNames.push_back(sdlExtensions[i]);
    }

    extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
 
    constexpr vk::ApplicationInfo appInfo{"Mini Renderer",
                                          VK_MAKE_VERSION(1, 0, 0),
                                          "No Engine",
                                          VK_MAKE_VERSION(1, 0, 0),
                                          vk::ApiVersion14};



 
    vk::InstanceCreateInfo createInfo{
        {},
        &appInfo,
        {},
        extensionNames
    };
    createInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    createInfo.ppEnabledLayerNames = validationLayers.data();

    std::vector<vk::ValidationFeatureEnableEXT> enabledFeatures = {
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation
    };
    vk::ValidationFeaturesEXT validationFeatures{};
    validationFeatures.setEnabledValidationFeatures(enabledFeatures);

    createInfo.pNext = &validationFeatures;

    this->instance = vk::raii::Instance(context, createInfo);


}