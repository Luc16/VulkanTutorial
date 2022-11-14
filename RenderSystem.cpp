//
// Created by luc on 13/11/22.
//

#include "RenderSystem.h"
namespace vtt {
    RenderSystem::RenderSystem(const Device& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout, const ShaderPaths& shaderPaths,
                                    const std::function<void(Pipeline::PipelineConfigInfo&)>& configurePipeline): m_deviceRef(device) {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass, shaderPaths, configurePipeline);
    }

    RenderSystem::~RenderSystem() {
        vkDestroyPipelineLayout(m_deviceRef.device(), m_pipelineLayout, nullptr);
    }

    void RenderSystem::bind(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet) {
        m_pipeline->bind(commandBuffer);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout,
                                0, 1, descriptorSet, 0, nullptr);
    }

    void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(m_deviceRef.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

    }

    void RenderSystem::createPipeline(VkRenderPass renderPass, const ShaderPaths& shaderPaths,
                                      const std::function<void(Pipeline::PipelineConfigInfo&)>& configurePipeline) {
        auto pipelineConfigInfo = Pipeline::defaultConfigInfo(m_pipelineLayout, renderPass);

        if (configurePipeline) configurePipeline(pipelineConfigInfo);

        m_pipeline = std::make_unique<Pipeline>(m_deviceRef, shaderPaths.vertPath,
                                                     shaderPaths.fragPath, pipelineConfigInfo);
    }
}

