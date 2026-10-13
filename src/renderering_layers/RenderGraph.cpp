#include "RenderGraph.hpp"
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <cstddef>
#include <unordered_map>
#include <vulkan/vulkan_raii.hpp>


//  Compilacion del RenderGraph,transforma descripciones declarativas a pipelines ejecutables
//  Este metodo realiza analisis de dependencias, asignacion de recursos y planeacion de ejecucion
void RenderGraph::compile(vk::raii::Device& device,VmaAllocator allocator){
    // Se limpian las ordenes de ejecucion por si se ejecuta más de una vez el compile no se dupliquen los passes
    executionOrder.clear();

    //Creacion del Grafo de dependencias
    //Construye relaciones de dependencias bidireccionales entre passes
    std::vector<std::vector<size_t>> dependencies(passes.size());   // De lo que cada pass depende
    std::vector<std::vector<size_t>> dependents(passes.size());     // Lo que depende de cada pass

    // Esto se usa para traquear cual pass produce cada recurso (dependencias read-after-write)
    std::unordered_map<std::string, size_t> resourceWriters;

    //  Registro de la produccion de recursos
    //  Registra quien produce cada recurso, el orden en que se generan los datos, 
    // no tiene porque ser el mismo orden en el que fluyen los datos y se consumen 
    for (size_t i = 0; i < passes.size(); ++i) {
        for (const auto& output : passes[i].outputs) {
            resourceWriters[output] = i;                    // Registra este pass como un productor
        }
    }


    //  Descubrimiento de dependencias a travez de el analisis del uso de recursos
    //  Ahora que cada productor es conocido, hay que analizar cada entrada de los passes para determinar las relaciones de los flujo de datos
    //  sin importar donde se encuentra el productor en el vector de passes.

    for (size_t i = 0; i < passes.size(); ++i) {
        const auto& pass = passes[i];

        // Procesa las dependencias de entrada, este pass debe esperar a los productores
        for (const auto& input : pass.inputs) {
            auto it = resourceWriters.find(input);
            if (it != resourceWriters.end() && it->second != i) {
                //  Se encontro un pass que produce esta entrada  - se crea un link de dependencia
                dependencies[i].push_back(it->second);      // Este pass depende de el productor
                dependents[it->second].push_back(i);        // Productor tiene esto como un dependiente
            }
        }
    }

    //  Clasificacion Topologica para un orden de ejecucion optimo
    //  Usa depth-first search para procesar secuencias de ejecuciones validas mientras detecta ciclos
    std::vector<bool> visited(passes.size(), false);       // Traquea nodos completados
    std::vector<bool> inStack(passes.size(), false);       // Traquea el camino de recursion actual


    std::function<void(size_t)> visit = [&](size_t node) {
            if (inStack[node]) {
                // Deteccion de Ciclo, se encontro una dependencia circular
                throw std::runtime_error("Ciclo detectado en el RenderGraph");
            }

            if (visited[node]) {
                return;  // A este nodo ya ha sido procesado junto con sus dependencias 
            }

            inStack[node] = true;   // Marca el nodo como "Siendo procesado actualmente"

            // Procesa de manera recursiva todos los passes de los cuales este depende (recorrido post-order)
            // para que el productor siempre se empujado en el executionOrder antes que sus consumidores
            for (auto dependency : dependencies[node]) {
                visit(dependency);
            }

            inStack[node] = false;  // Remueve de el camino actual
            visited[node] = true;   // lo marca como completamente procesado
            executionOrder.push_back(node);  // se agrega a la secuencia de ejecucion
        };

        // Procesa todos los nodos no visitados para manejar componentes del graph desconectados
        for (size_t i = 0; i < passes.size(); ++i) {
            if (!visited[i]) {
                visit(i);
            }
        }

        //  Sincronizacion automatica de la creacion de objetos
        //  Genera semaforos para todas las dependencias identificadas durante el analisis
        for(size_t i = 0; i < passes.size();i++) {
            for(auto dep : dependencies[i]) {
                // Crea un semaforo de GPU para la relacion de dependencias
                // El pass dependiente va a esperar en su semaforo antes de ejecutarse
                semaphores.emplace_back(device.createSemaphore({}));
                semaphoreSignalWaitPairs.emplace_back(dep, i); // par (Productor, consumidor)
            }
        }

        //  Creacion y asignacion de recursos fisicos
        //  Transforma descripciones de recursos en objetos de la GPU
        for(auto& [name,graphResource] : graphResources){
            //Configura la creacion de parametros de imagenes basandose en la descripcion de recursos
            vk::ImageCreateInfo imageInfo;
            imageInfo.setImageType(vk::ImageType::e2D);
            imageInfo.setFormat(graphResource.format);
            //  Image Info pide un Extent3D, por lo que creo uno usando el 2D y colocando un depth de 1
            imageInfo.setExtent(vk::Extent3D{graphResource.extent.width,graphResource.extent.height,1}); // Objetivo de texturizado/renderizado 2D
            imageInfo.setMipLevels(1);      // Una sola layer (sin array texture)
            imageInfo.setArrayLayers(1);
            imageInfo.setSamples(vk::SampleCountFlagBits::e1);    // Sin MSAA
            imageInfo.setTiling(vk::ImageTiling::eOptimal);     // layout de memoria de GPU optima
            imageInfo.setUsage(graphResource.usage);             // Flags de uso desde el registro
            imageInfo.setInitialLayout(vk::ImageLayout::eUndefined);    //  Layout inicial la cual sera transicionada

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; // render targets grandes
            graphResource.image = GpuImage(allocator,imageInfo,allocInfo);


            //Crear la ImageView para el acceso del shader
            vk::ImageViewCreateInfo viewInfo;
            viewInfo.setImage(graphResource.image.get());
            viewInfo.setViewType(vk::ImageViewType::e2D);
            viewInfo.setFormat(graphResource.format);
            viewInfo.setSubresourceRange({aspectFromFormat(graphResource.format), 0, 1, 0 , 1}); // Full Image Access


            graphResource.view = device.createImageView(viewInfo);
        }
}

// Esto es para seleccionar el tipo de aspecto depenciendo del formato del pass
vk::ImageAspectFlags RenderGraph::aspectFromFormat(vk::Format format) {
    switch (format) {
        case vk::Format::eD16Unorm:
        case vk::Format::eD32Sfloat:
        case vk::Format::eX8D24UnormPack32:
            return vk::ImageAspectFlagBits::eDepth;
        case vk::Format::eD16UnormS8Uint:
        case vk::Format::eD24UnormS8Uint:
        case vk::Format::eD32SfloatS8Uint:
            return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
        case vk::Format::eS8Uint:
            return vk::ImageAspectFlagBits::eStencil;
        default:
            return vk::ImageAspectFlagBits::eColor;
    }
}
