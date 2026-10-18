#include "vulkan/vulkan.hpp"
#include <vulkan/vulkan.h>

// Configuraciones del renderer
// TODO: Para despues hay que cambiar esto para que las configuraciones esten guardadas en un JSON, txt o cualqiuer otra forma
// Y crear una funcion que rellene este struct con esas configuraciones
struct RendererConfig {
    vk::Extent2D extent;
    bool     vsync = true;
    // framesInFlight, formato preferido de swapchain, etc.
};