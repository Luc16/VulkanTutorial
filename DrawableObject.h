//
// Created by luc on 13/11/22.
//

#ifndef VULKANTUTORIAL_DRAWABLEOBJECT_H
#define VULKANTUTORIAL_DRAWABLEOBJECT_H

#include <utility>

#include "Model.h"
#include "Texture.h"

namespace vtt {
    class DrawableObject {
    public:

        explicit DrawableObject(std::shared_ptr<vtt::Model> model, std::shared_ptr<vtt::Texture> texture = nullptr):
            m_model(std::move(model)), m_texture(std::move(texture)) {}

        DrawableObject(const DrawableObject &) = delete;
        DrawableObject &operator=(const DrawableObject &) = delete;
        ~DrawableObject() = default;

        void translate(glm::vec3 move) { m_translation += move; }
        void rotateAxis(int axis, float angle) { m_rotation[axis] += angle;}
        void resetRotation() {m_rotation *= 0.0f; }
        void render(VkCommandBuffer commandBuffer);

        [[nodiscard]] glm::mat4 modelMatrix() const;
        [[nodiscard]] VkDescriptorImageInfo textureInfo() const { return m_texture->descriptorInfo();}
    private:
        glm::vec3 m_translation{};
        glm::vec3 m_scale{1.f, 1.f, 1.f};
        glm::vec3 m_rotation{};

        std::shared_ptr<vtt::Model> m_model;
        std::shared_ptr<vtt::Texture> m_texture;
    };
}


#endif //VULKANTUTORIAL_DRAWABLEOBJECT_H
