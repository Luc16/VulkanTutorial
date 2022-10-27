//
// Created by luc on 22/10/22.
//

#ifndef VULKANTUTORIAL_SWAPCHAIN_H
#define VULKANTUTORIAL_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vector>
#include "utils.h"
#include "Device.h"
#include "Image.h"
#include <GLFW/glfw3.h>


namespace vtt {
    class SwapChain {
    public:

        SwapChain(const Device& device, VkExtent2D windowExtent);

        ~SwapChain();

        SwapChain(const SwapChain &) = delete;

        SwapChain &operator=(const SwapChain &) = delete;


        [[nodiscard]] VkSwapchainKHR get() const { return m_swapChain; }
        [[nodiscard]] uint32_t imageCount() const { return m_imgCount; }
        [[nodiscard]] uint32_t numImages() const { return m_swapChainImages.size(); }
        [[nodiscard]] uint32_t queueFamily() const { return m_queueFamily; }
        [[nodiscard]] VkFormat format() const { return m_imageFormat; }
        [[nodiscard]] VkRenderPass renderPass() const { return m_renderPass; }
        [[nodiscard]] VkExtent2D extent() const { return m_swapChainExtent; }
        [[nodiscard]] uint32_t width() const { return m_swapChainExtent.width; }
        [[nodiscard]] uint32_t height() const { return m_swapChainExtent.height; }
        [[nodiscard]] std::vector<VkImageView> imageViews() const { return m_swapChainImageViews; }
        [[nodiscard]] VkImageView imageViews(uint32_t i) const { return m_swapChainImageViews[i]; }
        [[nodiscard]] size_t numImageViews() const { return m_swapChainImageViews.size(); }
        [[nodiscard]] std::vector<VkFramebuffer> frameBuffers() const { return m_swapChainFramebuffers; }
        [[nodiscard]] VkFramebuffer frameBuffers(uint32_t i) const { return m_swapChainFramebuffers[i]; }
        [[nodiscard]] size_t numFrameBuffers() const { return m_swapChainFramebuffers.size(); }
        void resizeFrameBuffer(size_t newSize) { return m_swapChainFramebuffers.resize(newSize); }



    private:
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createDepthResources();
        void createColorResources();
        void createFrameBuffers();

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        VkFormat findDepthFormat();

        VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
                            VkFormatFeatureFlags features);


        VkSwapchainKHR m_swapChain{};

        uint32_t m_imgCount{};
        uint32_t m_queueFamily{};
        VkFormat m_imageFormat{};
        VkExtent2D m_swapChainExtent{};
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        const Device& m_deviceRef;
        VkExtent2D m_windowExtent;
        VkRenderPass m_renderPass{};

        std::unique_ptr<Image> m_colorImage;
        std::unique_ptr<Image> m_depthImage;
    };
}

#endif //VULKANTUTORIAL_SWAPCHAIN_H
