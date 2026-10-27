#pragma once
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <glm/gtc/matrix_transform.hpp>


class Camera{
    public:

        enum class CameraMovement {
            FORWARD,
            BACKWARD,
            LEFT,
            RIGHT,
            UP,
            DOWN
        };

        void updateCameraVectors();

        
        Camera(
            glm::vec3 position = glm::vec3(0.0f,0.0f,0.0f),
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
            float yaw = -90.0f,
            float pitch = 0.0f 
        );

        // Generacion de matrices para la integracion al pipeline grafico
        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;

        
        
        // Metodos para el procesamiento de inputs para diferentes modalidades de interaccion
        void processKeyboard(CameraMovement direction, float deltaTime);
        void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
        // Aun no se que hacer con el scroll del mouse
        void processMouseScroll(float yOffset);

        glm::vec3 getPosition() const { return position; }
        glm::vec3 getFront() const { return front; }
        float getZoom() const { return zoom; }


    private:
        // Posicionamiento espacial y vectores de orientacion
        glm::vec3 position; // La posicion de la caamara en  coordenadas de mundo
        glm::vec3 front;    // Direccion hacia al frente de la camara (Hacia donde mira la camara)
        glm::vec3 up;       // La direccion hacia arriba de la camara (La normal de la camara)
        glm::vec3 right;    // La direccion a a la derecha de la camara (perpendicular a el frente y arriba)
        glm::vec3 worldUp;  // Vector hacia arriba global de referencia (tipicamente el axis Y)

        // Representacion de la rotacion usando angulos de euler
        // Da un control intuitivo de como manejar las complejudades de la rotacion de la camara
        float yaw;          // Rotacion horizontal alrededor de axis worldUp (mirar de izquerda a derecha)
        float pitch;        // Rotacion vertical alrededor de el axiis derecho de la camara (el mirar hacia arriba y abajo)

        float movementSpeed;    // unidades por segundo para el movimiento de translacion
        float mouseSensitivity; // Multiplicador para el input de mouse para la rotacion
        float zoom;         // Area en la que alcanza a ver la camara para la projeccion de la perspectiva

        

};