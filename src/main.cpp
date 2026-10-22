#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>

#include "RenderDevice.hpp"
#include "Renderer.hpp"

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Rendering test", 1280, 720,
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow fallo: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    try {
        // Bloque propio: todo lo de Vulkan se destruye ANTES que la ventana
        // (la surface no puede sobrevivir a la ventana).
        RenderDevice renderDevice;
        renderDevice.init(window);
        
        int w = 1280, h = 720;
        SDL_GetWindowSizeInPixels(window, &w, &h);

        RendererConfig config{};
        config.extent = vk::Extent2D{static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
        config.vsync  = true;

        Renderer renderer(renderDevice, config);

        bool running = true;
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) running = false;
            }

            int w = 0, h = 0;
            SDL_GetWindowSizeInPixels(window, &w, &h);
            if (w == 0 || h == 0) {      // minimizada: no renderizar
                SDL_Delay(10);
                continue;
            }

            renderer.RenderFrame(renderDevice,
                                 vk::Extent2D{static_cast<uint32_t>(w),
                                              static_cast<uint32_t>(h)});
        }

        // La GPU puede seguir ejecutando frames: esperar antes de destruir nada
        renderDevice.getLogicalDevice().get().waitIdle();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}