//
// Created by luc on 23/10/22.
//

#ifndef VULKANTUTORIAL_BUFFER_H
#define VULKANTUTORIAL_BUFFER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "utils.h"
#include <GLFW/glfw3.h>

namespace vtt {
    class Buffer {
    public:

        Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;

        [[nodiscard]] VkBuffer get() const { return buffer; }

        void destroy(VkDevice device);
        void map(VkDevice device);
        void unmap(VkDevice device);
        void write(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void singleWrite(VkDevice device, void* data);


    private:
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        void* mapped = nullptr;

        VkDeviceSize bufferSize;
    };
}



#endif //VULKANTUTORIAL_BUFFER_H
