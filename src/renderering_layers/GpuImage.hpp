#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>


//  Clase Wrapper para Guardar la Imagen que va a la GPU
//  Razon para crearla: VMA no es compatible con vk::raii, por lo que se necesita crear una clase que use los VkImage estaticos corrientes
//  Y para ello necesito una clase que vea cuando se crea, cuando se elimina, las reglas de este para que no sea copiable pero si se pueda mover
//  Osea, Imitar el comportamiento que tiene el vk::raii en una clase que si es compatible con VMA

class GpuImage {
public:
    // Honestamente, ni idea
    GpuImage() = default;

    //  Constructor de la clase
    GpuImage(VmaAllocator allocator,
             const vk::ImageCreateInfo& imageInfo,
             const VmaAllocationCreateInfo& allocInfo)
        : allocator(allocator)
    {
        VkImageCreateInfo cImageInfo = imageInfo;  // vk::ImageCreateInfo se convierte implícito al struct C
        VkImage rawImage;

        vk::Result result = static_cast<vk::Result>(
            vmaCreateImage(allocator, &cImageInfo, &allocInfo, &rawImage, &allocation, nullptr)
        );
        if (result != vk::Result::eSuccess) {
            throw std::runtime_error("vmaCreateImage falló");
        }
        image = rawImage;
    }

    // Voy a ser honesto aca, no tengo ni idea como funcionan estas restricciones, yo solo las copie de una pagina que explicaba como manejar clases en C++

    // No copiable — solo un dueño a la vez (al igual que vk::raii)
    GpuImage(const GpuImage&) = delete;
    GpuImage& operator=(const GpuImage&) = delete;

    // Que la clase sea movible
    GpuImage(GpuImage&& other) noexcept {
        *this = std::move(other);
    }
    GpuImage& operator=(GpuImage&& other) noexcept {
        if (this != &other) {
            destroy();
            allocator  = other.allocator;
            image      = other.image;
            allocation = other.allocation;

            other.allocator  = nullptr;
            other.image      = nullptr;
            other.allocation = nullptr;
        }
        return *this;
    }
    //la funcion que destruye la clase
    ~GpuImage() {
        destroy();
    }

    vk::Image get() const { return image; }

private:
    void destroy() {
        if (image && allocation) {
            vmaDestroyImage(allocator, image, allocation);
        }
    }

    VmaAllocator   allocator  = nullptr;    //  El programa que asigna el espacio de la memoria que ocupara la imagen
    vk::Image      image      = nullptr;    //  La imagen en si
    VmaAllocation  allocation = nullptr;    //  La localizacion de donde se asigno la memoria y cuanto espacio ocupa
};