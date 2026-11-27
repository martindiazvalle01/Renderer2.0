#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <mutex>
#include <stdexcept>

// Clase  Hecha para subir command buffers a la commandPool
class QueueSubmitter {
public:
    QueueSubmitter() = default;

    QueueSubmitter(const QueueSubmitter&) = delete;
    QueueSubmitter& operator=(const QueueSubmitter&) = delete;

    void init(const vk::raii::Device& dev, const vk::raii::Queue& q) {
        device = &dev;
        queue  = &q;
    }

    template <class F>
    void submitAndWait(vk::raii::CommandPool& pool, F&& record)
    {
        vk::raii::CommandBuffer cmd = std::move(vk::raii::CommandBuffers(*device, {
            *pool,
            vk::CommandBufferLevel::ePrimary,
            1 }).front());
        vk::raii::Fence fence(*device, vk::FenceCreateInfo{});

        cmd.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        record(cmd);
        cmd.end();

        vk::CommandBufferSubmitInfo cmdInfo{};
        cmdInfo.setCommandBuffer(*cmd);

        vk::SubmitInfo2 si{};
        si.setCommandBufferInfos(cmdInfo);

        {
            std::lock_guard lock(mutex);
            queue->submit2(si, *fence);
        }
        if (device->waitForFences(*fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
            throw std::runtime_error("waitForFences fallo");
    }

private:
    const vk::raii::Device* device = nullptr;
    const vk::raii::Queue*  queue  = nullptr;
    std::mutex mutex;
};