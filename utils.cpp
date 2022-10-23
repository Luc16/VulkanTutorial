//
// Created by luc on 22/10/22.
//

#include "utils.h"

namespace vtt {

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice, VkSurfaceKHR surface) {
        QueueFamilyIndices familyIndices;
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& iQueueFamily : queueFamilies){
            if (iQueueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                familyIndices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(pDevice, i, surface, &presentSupport);
            if (presentSupport) familyIndices.presentFamily = i;

            if (familyIndices.isComplete()) break;

            i++;
        }

        return familyIndices;
    }

    VkImageView createImageView(VkDevice lDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(lDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }
}