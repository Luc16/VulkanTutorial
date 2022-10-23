//
// Created by luc on 22/10/22.
//

#ifndef VULKANTUTORIAL_UTILS_H
#define VULKANTUTORIAL_UTILS_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <array>
#include <chrono>
#include <memory>
#include <cstring>

namespace vtt {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily{};
        std::optional<uint32_t> presentFamily{};


        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    // TODO these will be in the device class

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice, VkSurfaceKHR surface);
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);


    VkImageView createImageView(VkDevice lDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    // TODO these will be in the device class

    void executeSingleCommand(VkDevice device, VkCommandPool commandPool, VkQueue queue, const std::function<void(VkCommandBuffer&)>& function);


    void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer,
                                   VkImage image, uint32_t width, uint32_t height);

    void createBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size,
                              VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

}


#endif //VULKANTUTORIAL_UTILS_H
