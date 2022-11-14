//
// Created by luc on 13/11/22.
//

#ifndef VULKANTUTORIAL_RENDERSYSTEM_H
#define VULKANTUTORIAL_RENDERSYSTEM_H

#include <utility>

#include "utils.h"
#include "Device.h"
#include "Pipeline.h"

namespace vtt {
    class RenderSystem {
    public:
        struct ShaderPaths {
            ShaderPaths(std::string  vertShaderPath, std::string  fragShaderPath): vertPath(std::move(vertShaderPath)), fragPath(std::move(fragShaderPath)){}
            const std::string fragPath;
            const std::string vertPath;
        };

        explicit RenderSystem(const Device &device);
        ~RenderSystem();
        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        [[nodiscard]] VkPipelineLayout pipelineLayout() { return m_pipelineLayout; }
        void bind(VkCommandBuffer commandBuffer, VkDescriptorSet* descriptorSet);

        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout, uint32_t pushConstantSize);
        void createPipeline(VkRenderPass renderPass, const ShaderPaths& shaderPaths, const std::function<void(Pipeline::PipelineConfigInfo&)>& configurePipeline = {});
    private:

        const Device& m_deviceRef;

        std::unique_ptr<Pipeline> m_pipeline;
        VkPipelineLayout m_pipelineLayout{};

        bool m_layoutCreated = false;
    };
}



#endif //VULKANTUTORIAL_RENDERSYSTEM_H
