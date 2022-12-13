//
// Created by luc on 12/12/22.
//

#ifndef VULKANTUTORIAL_INSTANCEDOBJECTS_H
#define VULKANTUTORIAL_INSTANCEDOBJECTS_H

#include "DrawableObject.h"

namespace vtt {
    template<typename InstanceData>
    class InstancedObjects : public DrawableObject, public std::vector<InstanceData> {
    public:
        InstancedObjects(Device &device, size_t initialSize, std::shared_ptr<vtt::Model> model,
                         std::shared_ptr<vtt::Texture> texture = nullptr);

        void render(vtt::RenderSystem &renderSystem, VkCommandBuffer commandBuffer) override;
        void updateBuffer();

        VkVertexInputBindingDescription getBindingDescription();

    private:
        void createInstanceBuffer(const Device &device);

        std::unique_ptr<vtt::Buffer> m_instanceBuffer;
    };


    template<typename InstanceData>
    InstancedObjects<InstanceData>::InstancedObjects(Device &device, size_t initialSize,
                                                     std::shared_ptr<vtt::Model> model,
                                                     std::shared_ptr<vtt::Texture> texture):
            DrawableObject(model, texture), std::vector<InstanceData>(initialSize) {
        createInstanceBuffer(device);
    }

    template<typename InstanceData>
    void InstancedObjects<InstanceData>::updateBuffer() {
        m_instanceBuffer->singleWrite(this->data());
    }

    template<typename InstanceData>
    void InstancedObjects<InstanceData>::render(vtt::RenderSystem &renderSystem, VkCommandBuffer commandBuffer) {
        if (renderSystem.pushConstantSize() > 0) {
            PushConstantData push{};
            push.modelMatrix = modelMatrix();
            vkCmdPushConstants(
                    commandBuffer,
                    renderSystem.pipelineLayout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(PushConstantData),
                    &push);
        }
        m_model->bind(commandBuffer);

        VkBuffer instanceBuffer = {m_instanceBuffer->getBuffer()};
        VkDeviceSize offsets[] = {0};

        vkCmdBindVertexBuffers(commandBuffer, 1, 1, &instanceBuffer, offsets);

        m_model->draw(commandBuffer, this->size());
    }

    template<typename InstanceData>
    void InstancedObjects<InstanceData>::createInstanceBuffer(const Device &device) {
        VkDeviceSize bufferSize = sizeof(this[0]) * this->size();

        vtt::Buffer stagingBuffer(device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.singleWrite(this->data());

        m_instanceBuffer = std::make_unique<vtt::Buffer>(device, bufferSize,
                                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                                         VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        device.copyBuffer(stagingBuffer.getBuffer(), m_instanceBuffer->getBuffer(), bufferSize);
    }

    template<typename InstanceData>
    VkVertexInputBindingDescription InstancedObjects<InstanceData>::getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(InstanceData);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescription;
    }


}
#endif //VULKANTUTORIAL_INSTANCEDOBJECTS_H
