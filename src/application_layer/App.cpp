#include "App.hpp"
#include <Renderer.hpp>
#include <SDL3/SDL_events.h>
#include <structTypes.hpp>
#include "RendererConfig.hpp"
#include "vulkan/vulkan.hpp"
#include "Cube.hpp"

void App::init(SDL_Window* window){
    coreLayer.init(window);

    RendererConfig config{};
    config.extent = vk::Extent2D{1280,720};
    renderer.init(coreLayer, config);
    FileContext context{};
    context.dirPath = "assets/models/props/DamagedHelmet";
    context.filePath = "assets/models/props/DamagedHelmet/DamagedHelmet.gltf";
    resourceManager.load(context);

    Model modelo = resourceManager.getModel(context);
    
    Cube cube(1.0f);
    std::vector<Vertex>   vertices = cube.getVertices();
    std::vector<uint32_t> indices  = cube.getIndices();

    for(auto& hMesh : modelo.meshes){
        Mesh* mesh = resourceManager.GetAssetManager().GetMeshPool().Get(hMesh);
        renderer.uploadMeshToGpu(mesh->indices,mesh->vertices);
        
    }

    
}

void App::run(SDL_Window* window){
    bool running = true;
    Uint64 lastTime = SDL_GetTicksNS();

    

    while (running) {
        Uint64 now = SDL_GetTicksNS();
        float deltaTime = (now - lastTime) / 1'000'000'000.0f; // La division es para que pase de nano segundos a segundos
        lastTime = now;
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) running = false;
            if (e.type == SDL_EVENT_MOUSE_MOTION) {
                camera.processMouseMovement(e.motion.xrel, -e.motion.yrel);
            
            SDL_SetWindowRelativeMouseMode(window, true);
        }
        }


        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        if (w == 0 || h == 0) {          // ventana minimizada: no dibujar
            SDL_Delay(10);
            continue;
        }
        processInput(deltaTime);

       

        renderer.RenderFrame(coreLayer, vk::Extent2D{(uint32_t)w, (uint32_t)h},camera);
    }

    coreLayer.context().device.waitIdle();
    renderer.destroyMesh();

}

// App.cpp
void App::processInput(float deltaTime) {
    const bool* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_W])      camera.processKeyboard(Camera::CameraMovement::FORWARD,  deltaTime);
    if (keys[SDL_SCANCODE_S])      camera.processKeyboard(Camera::CameraMovement::BACKWARD, deltaTime);
    if (keys[SDL_SCANCODE_A])      camera.processKeyboard(Camera::CameraMovement::LEFT,     deltaTime);
    if (keys[SDL_SCANCODE_D])      camera.processKeyboard(Camera::CameraMovement::RIGHT,    deltaTime);
    if (keys[SDL_SCANCODE_SPACE])  camera.processKeyboard(Camera::CameraMovement::UP,       deltaTime);
    if (keys[SDL_SCANCODE_LSHIFT]) camera.processKeyboard(Camera::CameraMovement::DOWN,     deltaTime);
}
