#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include "Instance.hpp"
#include "vulkan/vulkan.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

void Instance::create() {
    // Número de extensiones de SDL
    unsigned int sdlExtensionCount = 0;
    const char* const* sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);

    if (sdlExtensions == nullptr) {
        throw std::runtime_error(std::string("SDL_Vulkan_GetInstanceExtensions falló: ") + SDL_GetError());
    }

    std::vector<const char*> extensionNames;
    extensionNames.reserve(sdlExtensionCount + 1); // +1 para la extensión de debug

    for (Uint32 i = 0; i < sdlExtensionCount; ++i) {
        extensionNames.push_back(sdlExtensions[i]);
    }

    extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
 
    constexpr vk::ApplicationInfo appInfo{
        "Mini Renderer",
        VK_MAKE_VERSION(1, 0, 0),
        "No Engine",
        VK_MAKE_VERSION(1, 0, 0),
        vk::ApiVersion14
    };

    // 1. Mantener activo Synchronization Validation
    std::vector<vk::ValidationFeatureEnableEXT> enabledFeatures = {
        vk::ValidationFeatureEnableEXT::eSynchronizationValidation
    };
    vk::ValidationFeaturesEXT validationFeatures{};
    validationFeatures.setEnabledValidationFeatures(enabledFeatures);

    // 2. Configurar syncval_message_extra_properties con vk::LayerSettingEXT
    const vk::Bool32 enableExtraProps = vk::True;
    
    vk::LayerSettingEXT layerSetting{};
    layerSetting.pLayerName   = "VK_LAYER_KHRONOS_validation";
    layerSetting.pSettingName = "syncval_message_extra_properties";
    layerSetting.type         = vk::LayerSettingTypeEXT::eBool32;
    layerSetting.valueCount   = 1;
    layerSetting.pValues      = &enableExtraProps;

    vk::LayerSettingsCreateInfoEXT layerSettingsCreateInfo{};
    layerSettingsCreateInfo.setSettings(layerSetting);

    // 3. Encadenar ambos structs en el pNext
    layerSettingsCreateInfo.pNext = &validationFeatures;

    // 4. Crear InstanceCreateInfo usando los helpers de C++
    vk::InstanceCreateInfo createInfo{
        {},
        &appInfo,
        validationLayers,
        extensionNames,
        &layerSettingsCreateInfo // Apunta al inicio de la cadena pNext
    };

    this->instance = vk::raii::Instance(context, createInfo);
}