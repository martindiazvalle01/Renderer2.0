#include "RenderGraph.hpp"
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>


//  Compilacion del RenderGraph,transforma descripciones declarativas a pipelines ejecutables
//  Este metodo realiza analisis de dependencias, asignacion de recursos y planeacion de ejecucion
void RenderGraph::Compile(vk::raii::Device& device,VmaAllocator allocator){
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

        //La parte de sincronizacion se dejara para 
        //  Sincronizacion automatica de la creacion de objetos
        //  Genera semaforos para todas las dependencias identificadas durante el analisis
        //for(size_t i = 0; i < passes.size();i++) {
        //    for(auto dep : dependencies[i]) {
                // Crea un semaforo de GPU para la relacion de dependencias
                // El pass dependiente va a esperar en su semaforo antes de ejecutarse
        //        semaphores.emplace_back(device.createSemaphore({}));
        //        semaphoreSignalWaitPairs.emplace_back(dep, i); // par (Productor, consumidor)
        //    }
        //}

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

// Esto es para seleccionar el tipo de aspecto depenciendo del formato del pass (No estoy seguro de que esto funcione)
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



void RenderGraph::AddResource(const std::string& name, vk::Format format, vk::Extent2D extent,
                    vk::ImageUsageFlags usage, vk::ImageLayout initialLayout,
                    vk::ImageLayout finalLayout){


    GraphResource graphResource;
    graphResource.name = name;
    graphResource.format = format;
    graphResource.extent = extent;
    graphResource.usage = usage;
    graphResource.initialLayout = initialLayout;
    graphResource.finalLayout = finalLayout;

    // Debido a que GPUImage no deja ser copiada, no podemos usar
    //renderGraphMap[key] = myResource;, por lo que hacemos simplemente un emplace move para mover la propiedad de la clase
    graphResources.try_emplace(name,std::move(graphResource));

}


void RenderGraph::setImportedImage(std::string& name, vk::Image& image, vk::raii::ImageView& imageView){
    graphResources[name].image.set(image);
    graphResources[name].view = std::move(imageView);
}

// Interfaz de registro de passes para definir las operaciones de renderizado y sus dependiencias
void RenderGraph::AddPass(const std::string& name,
            const std::vector<std::string>& inputs,
            const std::vector<std::string>& outputs,
            std::function<void(vk::raii::CommandBuffer&)> executeFunc){

    Pass pass;
    pass.name = name;
    pass.inputs = inputs;
    pass.outputs = outputs;
    pass.executeFunc = executeFunc;

    passes.push_back(pass);
}

// Motor de ejecucion del renderGraph - Coordina la ejecucion de passes con sincronizacion automatica
// Este metodo transforma el rendergraph compilado a trabajo real para la GPU
void RenderGraph::Execute(vk::raii::CommandBuffer& commandBuffer, vk::Queue queue) {
    // Gestion de los estados de ejecucion para la sincronizacion dinamica
    std::vector<vk::CommandBuffer> cmdBuffers;      //  Almacenamiento del command buffer
    std::vector<vk::Semaphore> waitSemaphores;      //  Dependencias de Sincronizacion para el pass actual
    std::vector<vk::PipelineStageFlags> waitStages; //  Etapas del pipeline en espera
    std::vector<vk::Semaphore> signalSemaphores;        //  Semaforos para señalar cuando pasar despues de cada Pass

    // Pass de ejecucion ordenado con manejo de dependencias automatico
    // Ejecuta cada pass en el orden calculado que garantize la seguridad de las dependencias

    for(auto passIdx : executionOrder){
        const auto& pass = passes[passIdx];

        //  Configuracion de Sincronizacion - Agarra las dependencias para el pass actual
        //  Determina lo que el pass debe esperar antes de ejecutarse
        waitSemaphores.clear();
        waitStages.clear();

        for(size_t i = 0; i < semaphoreSignalWaitPairs.size(); i++){
            if(semaphoreSignalWaitPairs[i].second == passIdx){
                // Este pass depende de que se complete otro pass
                waitSemaphores.push_back(*semaphores[i]);               // Espera a que se complete la dependencia
                waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);    // Espera al estado de la salida
            }
        }

        // Recolecta semaforos a los cuales este pass vaa a ser una señal para passes dependientes(osea los passes a los que les debe avisar que termino)
        signalSemaphores.clear();
        for(size_t i = 0; i < semaphoreSignalWaitPairs.size(); i++){
            if(semaphoreSignalWaitPairs[i].first == passIdx) {
                // Otrso passes dependen en que se complete este pass
                signalSemaphores.push_back(*semaphores[i]);
            }
        }

        //  Preparacion del commandBuffer y las transiciones de layout del recurso
        //  Activa el comando para registrar y transicionar recursos a sus layouts apropiados
        commandBuffer.begin({});

        //Input de Transicion de recursos a layout leibles por el shader
        for (const auto& input : pass.inputs){
            auto& graphResource = graphResources[input];
            
            vk::ImageMemoryBarrier barrier;
            barrier.setOldLayout(graphResource.initialLayout);
            barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
            // TODO: Revisar si hay que cambiar estos 2, se ven sospechosamente estaticos
            barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setImage(graphResource.image.get());
            barrier.setSubresourceRange({aspectFromFormat(graphResource.format), 0, 1, 0, 1});
            //  TODO: Revisar estos 2 tambien, se ven sospechosamente estaticos
            barrier.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite);
            barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

            //Inserta la barrera de pipeline para una segura transicion de layout 

            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eAllCommands,        // Que espere a todo el trabajo previo
                vk::PipelineStageFlagBits::eFragmentShader,     // Que active el acceso al fragment shader
                vk::DependencyFlagBits::eByRegion,           // Dependencia local-region
                0, nullptr,barrier);    //Solo la Barrera de Imagen

        }
        // Ejecucion de Passes, Ejecuta la logica de renderizado
        // llama la funcion de renderizado proveida por el usuario con un commandBuffer preparado
        pass.executeFunc(commandBuffer);

        // Transiciones del layout finales - prepara recursos para su uso subsecuente
        // Transiciona los recursos del output aa su layout final requerido

        for( const auto& output : pass.outputs){
            auto& graphResource = graphResources[output];

            vk::ImageMemoryBarrier barrier;
            //Raro que varios de estos sean estaticos. TODO: Revisar esto
            barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal);
            barrier.setNewLayout(graphResource.finalLayout);
            barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
            barrier.setImage(graphResource.image.get());
            barrier.setSubresourceRange({aspectFromFormat(graphResource.format),0, 1, 0, 1});
            barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);        
            barrier.setDstAccessMask(vk::AccessFlagBits::eMemoryWrite);

            //  Inserta la barrera final para la transicion del layout
            commandBuffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, 
                vk::PipelineStageFlagBits::eAllCommands,
                vk::DependencyFlagBits::eByRegion,
                0, nullptr, barrier
            );
        }

        //  Presentacion de comandos con la sincronizacion
        commandBuffer.end();

        vk::SubmitInfo submitInfo;
        // ni idea porque el static cast, en la documentacion de vulkan decia que lo usara asi
        submitInfo.setSignalSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()));
        submitInfo.setPWaitSemaphores(waitSemaphores.data());
        submitInfo.setPWaitDstStageMask(waitStages.data());
        submitInfo.setCommandBufferCount(1);
        submitInfo.setPCommandBuffers(&*commandBuffer);
        submitInfo.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()));
        submitInfo.setPSignalSemaphores(signalSemaphores.data());

        vk::Result result = queue.submit(1,&submitInfo,nullptr);

        //TODO: Pensar que hay que hacer aca si falla la ejecucion del rendergraph
        if(result != vk::Result::eSuccess){
            continue;
        }
        
    }
}



