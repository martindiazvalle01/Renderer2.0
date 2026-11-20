#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "App.hpp"



int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init falló: %s", SDL_GetError());
        return 1;
    }

    const int width = 1280, height = 720;
    SDL_Window* window = SDL_CreateWindow("Renderer 2.0", width, height,
                                          SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow falló: %s", SDL_GetError());
        return 1;
    }

    {
        App vulkanApp;
        vulkanApp.init(window);
        vulkanApp.run(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}