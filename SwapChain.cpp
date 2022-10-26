//
// Created by luc on 22/10/22.
//

#include "SwapChain.h"
namespace vtt {
    SwapChain::SwapChain(GLFWwindow *window, const Device& device): m_deviceRef(device) {
        Device::SwapChainSupportDetails swapChainSupport = m_deviceRef.querySwapChainSupport(m_deviceRef.physicalDevice());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

        m_imgCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && m_imgCount > swapChainSupport.capabilities.maxImageCount)
            m_imgCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = m_deviceRef.surface();
        createInfo.minImageCount = m_imgCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        Device::QueueFamilyIndices familyIndices = m_deviceRef.findQueueFamilies(m_deviceRef.physicalDevice());
        uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()};
        m_queueFamily = familyIndices.graphicsFamily.value();

        if (familyIndices.graphicsFamily != familyIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(m_deviceRef.device(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(m_deviceRef.device(), m_swapChain, &m_imgCount, nullptr);
        m_swapChainImages.resize(m_imgCount);
        vkGetSwapchainImagesKHR(device.device(), m_swapChain, &m_imgCount, m_swapChainImages.data());

        m_imageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;

        createImageViews();

    }
    SwapChain::~SwapChain() {
        for (auto imageView: m_swapChainImageViews) {
            vkDestroyImageView(m_deviceRef.device(), imageView, nullptr);
        }
        for (auto framebuffer : m_swapChainFramebuffers) {
            vkDestroyFramebuffer(m_deviceRef.device(), framebuffer, nullptr);
        }
        vkDestroySwapchainKHR(m_deviceRef.device(), m_swapChain, nullptr);
    }


    void SwapChain::createImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); ++i) {
            m_swapChainImageViews[i] = createImageView(m_deviceRef.device(), m_swapChainImages[i], m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT,
                                                       1);
        }
    }

    void SwapChain::createFrameBuffers(VkRenderPass renderPass, std::array<VkImageView, 2> images){
        m_swapChainFramebuffers.resize(numImageViews());
        for (uint32_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
            std::array<VkImageView, 3> attachments = {images[0], images[1], imageViews(i)};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = width();
            framebufferInfo.height = height();
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_deviceRef.device(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &availableFormat: availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return availableFormats[0];
    }

    VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes) {
        for (const auto &availablePresentMode: availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChain::chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
}