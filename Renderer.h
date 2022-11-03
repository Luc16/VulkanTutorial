//
// Created by luc on 03/11/22.
//

#ifndef VULKANTUTORIAL_RENDERER_H
#define VULKANTUTORIAL_RENDERER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "SwapChain.h"

namespace vtt {
    class Renderer {
    public:
        Renderer(Window& window, const Device& device);
        ~Renderer();
        Renderer(const Renderer &) = delete;
        Renderer &operator=(const Renderer &) = delete;

        [[nodiscard]] uint32_t imageCount() const { return m_swapChain->imageCount(); }
        [[nodiscard]] uint32_t numImages() const { return m_swapChain->numImages(); }
        [[nodiscard]] uint32_t queueFamily() const { return m_swapChain->queueFamily(); }

        [[nodiscard]] uint32_t currentFrame() const { return m_currentFrame; }
        [[nodiscard]] VkRenderPass renderPass() const { return m_swapChain->renderPass(); }
        [[nodiscard]] float getSwapChainAspectRatio() const {return (float) m_swapChain->width() / (float) m_swapChain->height(); }
        [[nodiscard]] VkCommandBuffer getCurrentCommandBuffer() const {
            if (!m_isFrameStarted) throw std::runtime_error("Cannot get command buffer when frame not in progress");
            return m_commandBuffers[m_currentFrame];
        }

        void runRenderPass(const std::function<void()>& function) const;
        void runFrame(const std::function<void(VkCommandBuffer&)>& function);
    private:
        void recreateSwapChain();
        void createCommandBuffers();
        void freeCommandBuffers();

        std::unique_ptr<vtt::SwapChain> m_swapChain;
        std::vector<VkCommandBuffer> m_commandBuffers;
        uint32_t m_currentFrame{0};
        uint32_t m_imageIndex{};
        bool m_isFrameStarted{false};

        const Device& m_deviceRef;
        Window& m_windowRef;
    };
}



#endif //VULKANTUTORIAL_RENDERER_H
