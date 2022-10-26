//
// Created by luc on 23/10/22.
//

#include "Buffer.h"

namespace vtt {

    Buffer::Buffer(const Device& device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties):
    m_bufferSize(bufferSize), m_deviceRef(device)
    {
        m_deviceRef.createBuffer(bufferSize, usage, properties, m_buffer, m_memory);
    }

    Buffer::~Buffer() {
        vkDestroyBuffer(m_deviceRef.device(), m_buffer, nullptr);
        vkFreeMemory(m_deviceRef.device(), m_memory, nullptr);
    }

    void Buffer::map() {
        if (m_mapped != nullptr)
            throw std::runtime_error("Trying to map m_mapped m_buffer m_memory!");
        vkMapMemory(m_deviceRef.device(), m_memory, 0, m_bufferSize, 0, &m_mapped);
    }

    void Buffer::unmap() {
        if (m_mapped == nullptr)
            throw std::runtime_error("Trying to unmap unmapped m_buffer m_memory!");
        vkUnmapMemory(m_deviceRef.device(), m_memory);
        m_mapped = nullptr;
    }

    void Buffer::write(void *data, VkDeviceSize size, VkDeviceSize offset) {
        if (m_mapped == nullptr)
            throw std::runtime_error("Trying to write to unmapped m_buffer m_memory!");

        if (size == VK_WHOLE_SIZE)
            memcpy(m_mapped, data, (size_t) m_bufferSize);
        else {
            char *memOffset = (char *)m_mapped;
            memOffset += offset;
            memcpy(m_mapped, data, (size_t) size);

        }
    }

    void Buffer::singleWrite(void *data) {
        map();
        write(data);
        unmap();
    }
}