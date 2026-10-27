#pragma once
#include "vulkan/vulkan.hpp"
#include <mutex>
#include <stdexcept>

#include <vulkan/vulkan_raii.hpp>
class QueueSubmitter {
    public:
        QueueSubmitter(const vk::raii::Device& device, const vk::raii::Queue& queue)
            : device(device), queue(queue) {}

        QueueSubmitter(const QueueSubmitter&) = delete;
        QueueSubmitter& operator=(const QueueSubmitter&) = delete;

        template <class F>
        void submitAndWait(vk::raii::CommandPool& pool, F&& record)
        {
            vk::raii::CommandBuffer cmd = std::move(vk::raii::CommandBuffers(device, {
                *pool,
                vk::CommandBufferLevel::ePrimary,
                 1 }).front());
            vk::raii::Fence fence(device, vk::FenceCreateInfo{});

            cmd.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
            record(cmd);
            cmd.end();

            vk::CommandBufferSubmitInfo cmdInfo{};
            cmdInfo.setCommandBuffer(*cmd);

            vk::SubmitInfo2 si{};
            si.setCommandBufferInfos(cmdInfo);   // copia count + puntero

            {   
                std::lock_guard lock(mutex);
                queue.submit2(si, *fence);
            }
            if (device.waitForFences(*fence, VK_TRUE, UINT64_MAX) != vk::Result::eSuccess)
                throw std::runtime_error("waitForFences fallo");
            // fence y cmd se destruyen/liberan aquí al salir del scope
        }

    private:
        const vk::raii::Device& device;
        const vk::raii::Queue&  queue;
        std::mutex mutex;
};