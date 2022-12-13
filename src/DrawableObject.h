//
// Created by luc on 13/11/22.
//

#ifndef VULKANTUTORIAL_DRAWABLEOBJECT_H
#define VULKANTUTORIAL_DRAWABLEOBJECT_H

#include <utility>

#include "Model.h"
#include "Texture.h"
#include "RenderSystem.h"

namespace vtt {
    class DrawableObject {
    public:
        struct PushConstantData {
            glm::mat4 modelMatrix{1.f};
        };

        explicit DrawableObject(std::shared_ptr<vtt::Model> model, std::shared_ptr<vtt::Texture> texture = nullptr):
            m_model(std::move(model)), m_texture(std::move(texture)) {}

        DrawableObject(const DrawableObject &) = delete;
        DrawableObject &operator=(const DrawableObject &) = delete;
        ~DrawableObject() = default;

        void setScale(float scale) { m_scale = glm::vec3(scale); }
        void translate(glm::vec3 move) { m_translation += move; }
        void rotateAxis(int axis, float angle) { m_rotation[axis] += angle;}
        void resetRotation(int axis) {m_rotation[axis] = 0; }
        virtual void render(vtt::RenderSystem& renderSystem, VkCommandBuffer commandBuffer);

        [[nodiscard]] glm::mat4 modelMatrix() const;
        [[nodiscard]] VkDescriptorImageInfo textureInfo() const { return m_texture->descriptorInfo();}
        glm::vec3 m_translation{};
    private:
        glm::vec3 m_scale{1.f, 1.f, 1.f};
        glm::vec3 m_rotation{};

        std::shared_ptr<vtt::Texture> m_texture;
    protected:
        std::shared_ptr<vtt::Model> m_model;
    };
}


#endif //VULKANTUTORIAL_DRAWABLEOBJECT_H
