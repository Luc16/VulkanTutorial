//
// Created by luc on 22/10/22.
//

#include "SwapChain.h"
namespace vtt {
    SwapChain::SwapChain(const Device& device, VkExtent2D windowExtent): m_deviceRef(device), m_windowExtent(windowExtent) {
        createSwapChain();
        createImageViews();
        createRenderPass();
        createColorResources();
        createDepthResources();
        createFrameBuffers();

    }
    SwapChain::~SwapChain() {
        for (auto imageView: m_swapChainImageViews) {
            vkDestroyImageView(m_deviceRef.device(), imageView, nullptr);
        }
        for (auto framebuffer : m_swapChainFramebuffers) {
            vkDestroyFramebuffer(m_deviceRef.device(), framebuffer, nullptr);
        }
        vkDestroyRenderPass(m_deviceRef.device(), m_renderPass, nullptr);
        vkDestroySwapchainKHR(m_deviceRef.device(), m_swapChain, nullptr);
    }

    void SwapChain::createSwapChain() {
        Device::SwapChainSupportDetails swapChainSupport = m_deviceRef.querySwapChainSupport(m_deviceRef.physicalDevice());

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

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
        vkGetSwapchainImagesKHR(m_deviceRef.device(), m_swapChain, &m_imgCount, m_swapChainImages.data());

        m_imageFormat = surfaceFormat.format;
        m_swapChainExtent = extent;
    }

    void SwapChain::createImageViews() {
        m_swapChainImageViews.resize(m_swapChainImages.size());

        for (size_t i = 0; i < m_swapChainImageViews.size(); ++i) {
            m_swapChainImageViews[i] = Image::createSwapChainImageView(m_deviceRef.device(), m_swapChainImages[i],
                                                                       m_imageFormat, VK_IMAGE_ASPECT_COLOR_BIT,1);
        }
    }

    void SwapChain::createRenderPass(){
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format();
        colorAttachment.samples = m_deviceRef.msaaSamples();
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = m_deviceRef.msaaSamples();
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = format();
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        subpass.pResolveAttachments = &colorAttachmentResolveRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(m_deviceRef.device(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS){
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void SwapChain::createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        m_depthImage = std::make_unique<Image>(m_deviceRef, width(), height(), 1, m_deviceRef.msaaSamples(), depthFormat,
                                               VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
//        m_colorImage->createImageView(depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
    }

    void SwapChain::createColorResources(){
        VkFormat colorFormat = format();

        m_colorImage = std::make_unique<Image>(m_deviceRef, width(), height(), 1,
                                               m_deviceRef.msaaSamples(), colorFormat, VK_IMAGE_TILING_OPTIMAL,
                                               VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
//        m_colorImage->createImageView(colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }

    void SwapChain::createFrameBuffers(){
        m_swapChainFramebuffers.resize(numImageViews());
        for (uint32_t i = 0; i < m_swapChainFramebuffers.size(); i++) {
            std::array<VkImageView, 3> attachments = {m_colorImage->view(), m_depthImage->view(), imageViews(i)};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
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

    VkFormat SwapChain::findDepthFormat() {
        return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                                   VK_IMAGE_TILING_OPTIMAL,
                                   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat SwapChain::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        VkFormatProperties props;
        for (VkFormat format: candidates){
            vkGetPhysicalDeviceFormatProperties(m_deviceRef.physicalDevice(), format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features
                || tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
                return format;
        }
        throw std::runtime_error("failed to find supported format!");
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

    VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        } else {

            VkExtent2D actualExtent = m_windowExtent;

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                            capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                             capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
}