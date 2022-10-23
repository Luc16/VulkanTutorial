//
// Created by luc on 22/10/22.
//

#ifndef VULKANTUTORIAL_SWAPCHAIN_H
#define VULKANTUTORIAL_SWAPCHAIN_H

#include <vulkan/vulkan.h>
#include <vector>
#include "utils.h"
#include <GLFW/glfw3.h>


namespace vtt {
    class SwapChain {
    public:

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        SwapChain(GLFWwindow *window, VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

        SwapChain(const SwapChain &) = delete;

        SwapChain &operator=(const SwapChain &) = delete;

        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice pDevice, VkSurfaceKHR surface);

        [[nodiscard]] VkSwapchainKHR get() const { return swapChain; }
        [[nodiscard]] uint32_t imageCount() const { return imgCount; }
        [[nodiscard]] uint32_t numImages() const { return swapChainImages.size(); }
        [[nodiscard]] uint32_t queueFamily() const { return qFamily; }
        [[nodiscard]] VkFormat format() const { return imageFormat; }
        [[nodiscard]] VkExtent2D extent() const { return swapChainExtent; }
        [[nodiscard]] uint32_t width() const { return swapChainExtent.width; }
        [[nodiscard]] uint32_t height() const { return swapChainExtent.height; }
        [[nodiscard]] std::vector<VkImageView> imageViews() const { return swapChainImageViews; }
        [[nodiscard]] VkImageView imageViews(uint32_t i) const { return swapChainImageViews[i]; }
        [[nodiscard]] size_t numImageViews() const { return swapChainImageViews.size(); }
        [[nodiscard]] std::vector<VkFramebuffer> frameBuffers() const { return swapChainFramebuffers; }
        [[nodiscard]] VkFramebuffer frameBuffers(uint32_t i) const { return swapChainFramebuffers[i]; }
        [[nodiscard]] size_t numFrameBuffers() const { return swapChainFramebuffers.size(); }
        void resizeFrameBuffer(size_t newSize) { return swapChainFramebuffers.resize(newSize); }

        void destroySwapChain(VkDevice device);


        void createFrameBuffers(VkDevice device, VkRenderPass renderPass, std::array<VkImageView, 2> images);

    private:
        VkSwapchainKHR swapChain{};

        uint32_t imgCount{};
        uint32_t qFamily{};
        std::vector<VkImage> swapChainImages;
        VkFormat imageFormat{};
        VkExtent2D swapChainExtent{};
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> swapChainFramebuffers;

        void createImageViews(VkDevice device);

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        static VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities);
    };
}

#endif //VULKANTUTORIAL_SWAPCHAIN_H
