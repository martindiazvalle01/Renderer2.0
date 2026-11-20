#pragma once


#include <Camera.hpp>
#include <SDL3/SDL_video.h>
#include "RenderDevice.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"

class App{

    public:
        void init(SDL_Window* window);
        void run(SDL_Window* window);

        struct CameraData{
            glm::mat4 viewProjMatrix;
            glm::vec4 cameraPos;
        };

    private:
        

        RenderDevice coreLayer;
        ResourceManager resourceManager;
        Renderer renderer;
        Camera camera{};
        void processInput(float deltaTime);
};