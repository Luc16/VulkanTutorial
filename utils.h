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

namespace vtt {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily{};
        std::optional<uint32_t> presentFamily{};


        [[nodiscard]] bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice pDevice, VkSurfaceKHR surface);

    VkImageView createImageView(VkDevice lDevice, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
}


#endif //VULKANTUTORIAL_UTILS_H
