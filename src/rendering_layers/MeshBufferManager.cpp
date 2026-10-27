#include "MeshBufferManager.hpp"
#include "vulkan/vulkan.hpp"
#include <Allocator.hpp>
#include <Vertex.hpp>
#include <cstdint>
#include <cstring>
#include <span>

MeshBufferManager::GpuBuffer MeshBufferManager::createBuffer(uint64_t allocSize, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage,VmaAllocationCreateFlags flags){ 

    //Se asigna el buffer de memoria

    vk::BufferCreateInfo bufferInfo;
    bufferInfo.sType = vk::StructureType::eBufferCreateInfo;
    bufferInfo.pNext = nullptr;
    bufferInfo.size = allocSize;

    bufferInfo.usage = usage;

    VmaAllocationCreateInfo vmaAllocInfo = {};
    vmaAllocInfo.usage = memoryUsage;
    vmaAllocInfo.flags = flags;

    GpuBuffer newBuffer;

    VkBufferCreateInfo cBufferInfo = bufferInfo;   // vk::BufferCreateInfo -> VkBufferCreateInfo (conversión implícita)
    VkBuffer rawBuffer = VK_NULL_HANDLE;

    // Conversion de vulkan de C normal a C++
    vk::Result result = static_cast<vk::Result>(
        vmaCreateBuffer(allocator, &cBufferInfo, &vmaAllocInfo,
                    &rawBuffer, &newBuffer.allocation, &newBuffer.info)
    );
    if (result != vk::Result::eSuccess) {
        throw std::runtime_error("vmaCreateBuffer falló");
    }
    newBuffer.buffer = rawBuffer;                  // VkBuffer -> vk::Buffer

    return newBuffer;
}

void MeshBufferManager::destroyBuffer(GpuBuffer& buffer){
    vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    // Evita doble destruccion si se llama dos veces
    buffer.buffer = nullptr;
    buffer.allocation = nullptr;
}

MeshBufferManager::MeshBufferManager(VmaAllocator allocator, const vk::raii::Device& device,
                  const vk::raii::Queue& queue, uint32_t queueFamilyIndex)
    : allocator(allocator), device(device), queue(queue) , queueSubmitter(device,queue){

    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags = vk::CommandPoolCreateFlagBits::eTransient;
    poolInfo.queueFamilyIndex = queueFamilyIndex;
    uploadPool = vk::raii::CommandPool(device, poolInfo);
}

MeshBufferManager::GpuMeshBuffers MeshBufferManager::uploadMesh(std::span<uint32_t> indices, std::span<Vertex> vertices){
    // Calculamos el tamaño en bytes de los vertices e indices que seran subidos a la GPU
    const vk::DeviceSize vertexBufferSize = vertices.size_bytes();
    const vk::DeviceSize indexBufferSize  = indices.size_bytes();

    GpuMeshBuffers newSurface;
    newSurface.indexCount = static_cast<uint32_t>(indices.size());
    //Creamos el vertex buffer
    newSurface.vertexBuffer = createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		VMA_MEMORY_USAGE_GPU_ONLY);

    //Encontramos la direccion de el vertex buffer
    vk::BufferDeviceAddressInfo deviceAddressInfo;
    deviceAddressInfo.sType = vk::StructureType::eBufferDeviceAddressInfo;
    deviceAddressInfo.buffer = newSurface.vertexBuffer.buffer;
    newSurface.vertexBufferAddress = device.getBufferAddress(deviceAddressInfo); 

    // creamos el index buffer
    newSurface.indexBuffer = createBuffer(indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY);

    GpuBuffer staging = createBuffer(vertexBufferSize + indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, VMA_ALLOCATION_CREATE_MAPPED_BIT);

    void* data = staging.info.pMappedData;

    // Copiamos el vertex Buffer
    memcpy(data,vertices.data(), vertexBufferSize);
    // Copiamos el Index Buffer
    memcpy((char*)data + vertexBufferSize,indices.data() , indexBufferSize);

    queueSubmitter.submitAndWait(uploadPool, [&](vk::raii::CommandBuffer& cmd) {
        cmd.copyBuffer(staging.buffer, newSurface.vertexBuffer.buffer,
                       vk::BufferCopy{ 0, 0, vertexBufferSize });
        cmd.copyBuffer(staging.buffer, newSurface.indexBuffer.buffer,
                       vk::BufferCopy{ vertexBufferSize, 0, indexBufferSize });
    });

    // Destruimos el staging buffer porque es simplemente un lugar donde copiar los datos, una vez la GPU termina de copiar los datos que viven
    // en la RAM, no nos sirve de nada conservarlo
    destroyBuffer(staging);

    return newSurface;

}