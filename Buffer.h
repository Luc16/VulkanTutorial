//
// Created by luc on 23/10/22.
//

#ifndef VULKANTUTORIAL_BUFFER_H
#define VULKANTUTORIAL_BUFFER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "utils.h"
#include "Device.h"
#include <GLFW/glfw3.h>

namespace vtt {
    class Buffer {
    public:

        Buffer(const Device& device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        Buffer(const Buffer &) = delete;
        Buffer &operator=(const Buffer &) = delete;
        ~Buffer();

        [[nodiscard]] VkBuffer get() const { return m_buffer; }

        // TODO check alternatives for unmap
        void map();
        void unmap();
        void write(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
        void singleWrite(void* data);


    private:
        VkBuffer m_buffer = VK_NULL_HANDLE;
        VkDeviceMemory m_memory = VK_NULL_HANDLE;
        void* m_mapped = nullptr;

        VkDeviceSize m_bufferSize;
        const Device& m_deviceRef;
    };
}



#endif //VULKANTUTORIAL_BUFFER_H
