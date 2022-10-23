//
// Created by luc on 23/10/22.
//

#include "Buffer.h"

namespace vtt {

    Buffer::Buffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties): bufferSize(bufferSize) {
        createBuffer(device, physicalDevice, bufferSize, usage, properties, buffer, memory);
    }

    void Buffer::destroy(VkDevice device) {
        vkDestroyBuffer(device, buffer, nullptr);
        vkFreeMemory(device, memory, nullptr);
    }

    void Buffer::map(VkDevice device) {
        if (mapped != nullptr)
            throw std::runtime_error("Trying to map mapped buffer memory!");
        vkMapMemory(device, memory, 0, bufferSize, 0, &mapped);
    }

    void Buffer::unmap(VkDevice device) {
        if (mapped == nullptr)
            throw std::runtime_error("Trying to unmap unmapped buffer memory!");
        vkUnmapMemory(device, memory);
        mapped = nullptr;
    }

    void Buffer::write(void *data, VkDeviceSize size, VkDeviceSize offset) {
        if (mapped == nullptr)
            throw std::runtime_error("Trying to write to unmapped buffer memory!");

        if (size == VK_WHOLE_SIZE)
            memcpy(mapped, data, (size_t) bufferSize);
        else {
            char *memOffset = (char *)mapped;
            memOffset += offset;
            memcpy(mapped, data, (size_t) size);

        }
    }

    void Buffer::singleWrite(VkDevice device, void *data) {
        map(device);
        write(data);
        unmap(device);
    }
}