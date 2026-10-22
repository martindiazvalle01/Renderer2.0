#include "RenderGraph.hpp"
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <cstddef>
#include <stdexcept>
#include <unordered_map>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_raii.hpp>

// Por ahora el render Graph solo deberia funcionar con un color pass, falta hacer más pruebas


//  Compilacion del RenderGraph,transforma descripciones declarativas a pipelines ejecutables
//  Este metodo realiza analisis de dependencias, asignacion de recursos y planeacion de ejecucion
void RenderGraph::Compile(const vk::raii::Device& device,VmaAllocator allocator){
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
            if(graphResource.imported) continue;
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
        compiled = true;

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
                    vk::ImageLayout finalLayout, bool imported){


    GraphResource graphResource;
    graphResource.name = name;
    graphResource.format = format;
    graphResource.extent = extent;
    graphResource.usage = usage;
    graphResource.initialLayout = initialLayout;
    graphResource.finalLayout = finalLayout;
    graphResource.imported = imported;

    // Debido a que GPUImage no deja ser copiada, no podemos usar
    //renderGraphMap[key] = myResource;, por lo que hacemos simplemente un emplace move para mover la propiedad de la clase
    graphResources.try_emplace(name,std::move(graphResource));

    // el grafo cambió, hay que recompilar
    compiled = false;

}

// La imagen va a llegar importada de la swapchain
void RenderGraph::setImportedImage(const std::string& name, vk::Image image, vk::ImageView view) {
    auto& res = GetResourceChecked(name);
    res.image.set(image);
    res.view = view;                          // sin ownership, no toca ownedView
    res.currentLayout = vk::ImageLayout::eUndefined;
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
    // el grafo cambió, hay que recompilar
    compiled = false;
}


// Motor de ejecucion del renderGraph - Coordina la ejecucion de passes con sincronizacion automatica
// Este metodo transforma el rendergraph compilado a trabajo real para la GPU
void RenderGraph::Execute(vk::raii::CommandBuffer& commandBuffer) {
    if (!compiled){
        throw std::runtime_error("RenderGraph::Execute sin Compile() (o se modifico el grafo despues de compilar)");
    }
    // Si el recurso no es persistente, se reinicia su layout cada frame
    for (auto& [name, res] : graphResources) {
        if (!res.persistent && !res.imported)
            res.currentLayout = vk::ImageLayout::eUndefined;
    }


    // Pass de ejecucion ordenado con manejo de dependencias automatico
    // Ejecuta cada pass en el orden calculado que garantize la seguridad de las dependencias

    for(auto passIdx : executionOrder){
        if (!passes[passIdx].enabled) continue;
        const auto& pass = passes[passIdx];

        
        // Transiciona el layout de los recursos input
        for(const auto& input : pass.inputs){
            GraphResource& res = GetResourceChecked(input);
            TransitionResource(commandBuffer, res, vk::ImageLayout::eShaderReadOnlyOptimal);
        }
     

        //Transiciona el layout de los recursos output
        for(const auto& output : pass.outputs){
            GraphResource& res = GetResourceChecked(output);
            TransitionResource(commandBuffer, res, LayoutForWrite(res.format));
        }
        
        // Ejecucion de Passes, Ejecuta la logica de renderizado
        // llama la funcion de renderizado proveida por el usuario con un commandBuffer preparado
        pass.executeFunc(commandBuffer);
    }

    // Transiciona todos los recursos al layout final
    for (auto& [name, res] : graphResources) {
        if (res.finalLayout == vk::ImageLayout::eUndefined) continue;   // no se puede transicionar hacia Undefined
            if (res.currentLayout == res.finalLayout) continue;             // ya está donde debe
            TransitionResource(commandBuffer, res, res.finalLayout);
    }
    
    
}

// Busca el recurso y falla con un mensaje claro si no existe (reemplaza a graphResources[nombre])
RenderGraph::GraphResource& RenderGraph::GetResourceChecked(const std::string& name) {
    auto it = graphResources.find(name);
    if (it == graphResources.end()) {
        throw std::runtime_error("RenderGraph: recurso no registrado: " + name);
    }
    return it->second;
}

// Lleva el recurso al layout pedido y actualiza el seguimiento
void RenderGraph::TransitionResource(vk::raii::CommandBuffer& commandBuffer,
                                     GraphResource& resource, vk::ImageLayout newLayout) {
    // lectura -> lectura en el mismo layout: no hace falta barrera
    if (resource.currentLayout == newLayout &&
        newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        return;
    }

    TransitionImageLayout(commandBuffer, resource.image.get(), resource.format,
                          resource.currentLayout, newLayout);
    resource.currentLayout = newLayout;   // el recurso "recuerda" en que layout quedo
}

//TODO: Cambiar todo esto para que sea con switch, o buscar alguna forma de que se vea más ordenado la parte de las asignaciones, 
// como separar haciendo un if para el initial y que cada parte de la que empieze tenga una clase segun cual es el layout final 

// Implementacion de la transicion del layout de la imagen
void RenderGraph::TransitionImageLayout(vk::raii::CommandBuffer& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout){
    //Esto especifica el tipo de barrera que se usara para el viejo layout usando los datos del viejo layout
    BarrierHalf srcHalf = SrcHalf(oldLayout);
    //Esto especifica el tipo de barrera que se usara para el nuevo layout usando los datos del nuevo layout
    BarrierHalf dstHalf = DstHalf(newLayout);

    //  Configuracion basica de la estructura de la barrera de memoria de imagen
    //  Esta barrera va a coordenar el acceso de memoria y transiciones de layouts
    vk::ImageMemoryBarrier barrier;
    barrier.setOldLayout(oldLayout);                                                                        // Estado actual del layout
    barrier.setNewLayout(newLayout);                                                                        // Layout objetivo despues de la transicion
    barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);                                      // Ninguna family Queue es dueño de esta transferencia
    barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);                                      // La misma familia queue con la que se inicio
    barrier.setImage(image);
    barrier.setSrcAccessMask(srcHalf.access);
    barrier.setDstAccessMask(dstHalf.access);                                                                                     // Imagen objetivo para la transicion
    barrier.setSubresourceRange({aspectFromFormat(format),0,1,0,1});      // Rango completo del color de la imagen


    //  Ejecuta esta barera de pipeline con la configuracion de la sincronizacion
    //  Esto junta el layout de transicion y la sincronizacion de memoria con el commandBuffer
    commandBuffer.pipelineBarrier(srcHalf.stage,dstHalf.stage,vk::DependencyFlags{},
                                 0, nullptr, barrier);
}


RenderGraph::BarrierHalf RenderGraph::SrcHalf(vk::ImageLayout oldLayout){
    switch (oldLayout) {
        case vk::ImageLayout::eUndefined:
            return {vk::PipelineStageFlagBits::eTopOfPipe,vk::AccessFlagBits::eNone};
            break;
        
        case vk::ImageLayout::eTransferDstOptimal:
            return {vk::PipelineStageFlagBits::eTransfer,vk::AccessFlagBits::eTransferWrite};
            break;

        case vk::ImageLayout::eColorAttachmentOptimal:
            return {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite};
            break;
        
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return {vk::PipelineStageFlagBits::eFragmentShader,vk::AccessFlagBits::eShaderRead};
            break;
        
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            // el | es para indicar que puede ser cualquiera de los 2, osea guarda los 2
            // esto es para que la barrera haga qe nada avance hasta que estas 2 stages esten finalizadas
            return {vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite};
            break;
        default:
            throw std::invalid_argument("SrcHalf: layout no soportado");
    }
}

RenderGraph::BarrierHalf RenderGraph::DstHalf(vk::ImageLayout newLayout){
    switch (newLayout) {

        case vk::ImageLayout::eTransferDstOptimal:
            return {vk::PipelineStageFlagBits::eTransfer,vk::AccessFlagBits::eTransferWrite};
            break;

        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return{vk::PipelineStageFlagBits::eFragmentShader,vk::AccessFlagBits::eShaderRead};
            break;

        case vk::ImageLayout::eColorAttachmentOptimal:
            return {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite};
            break;

        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            // el | es para indicar que puede ser cualquiera de los 2, osea guarda los 2
            // esto es para que la barrera haga qe nada avance hasta que estas 2 stages esten finalizadas
            return {vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,
                    vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite};
                    break;

        case vk::ImageLayout::ePresentSrcKHR:
            return {vk::PipelineStageFlagBits::eBottomOfPipe,vk::AccessFlagBits{}};
            break;
            
        default:
            throw std::invalid_argument("DstHalf: layout no soportado");
    
    }
}

vk::ImageLayout RenderGraph::LayoutForWrite(vk::Format format){
    auto aspect = aspectFromFormat(format);
    bool isDepthStencil = aspect != vk::ImageAspectFlagBits::eColor;
    return  isDepthStencil ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
 
}


