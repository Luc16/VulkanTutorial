//
// Created by luc on 13/11/22.
//

#include "DrawableObject.h"

namespace vtt {

    void DrawableObject::render(VkCommandBuffer commandBuffer) {
        m_model->bind(commandBuffer);
        m_model->draw(commandBuffer);
    }

    glm::mat4 DrawableObject::modelMatrix() const {
        return glm::translate(glm::mat4(1.0f), m_translation)*glm::scale(glm::mat4(1.0f), m_scale)
               * glm::rotate(glm::mat4(1.0f), m_rotation[0],glm::vec3(0.0f, 0.0f, 1.0f))
               * glm::rotate(glm::mat4(1.0f), m_rotation[1],glm::vec3(0.0f, 1.0f, 0.0f))
               * glm::rotate(glm::mat4(1.0f), m_rotation[2],glm::vec3(1.0f, 0.0f, 0.0f));
    }
}