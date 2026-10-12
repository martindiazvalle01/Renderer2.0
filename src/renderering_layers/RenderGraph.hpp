
#include "vulkan/vulkan.hpp"
#include <string>



class RenderGraph{

    //  Descripcion del recurso y su estructura
    //  Representa el recurso de la imagen usada durante el renderizado (Texturas)
    struct GraphResource{
        std::string name;     //Nombre descriptivo para debugging      
        vk::Format format;    // Formato a nivel pixel (RGBA8, Depth24Stencil8,etc.)
        vk::Extent2D extent;    // Dimensiones en pixeles del recurso 2D (Osea el tamaño de la textura)
        vk::ImageUsageFlags usage;  // Como va a ser usado este recurso
    };
};