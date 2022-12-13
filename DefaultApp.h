//
// Created by luc on 13/12/22.
//

#ifndef VULKANTUTORIAL_DEFAULTAPP_H
#define VULKANTUTORIAL_DEFAULTAPP_H

#include <sstream>
#include "external/imgui/imgui.h"
#include "external/objloader/tiny_obj_loader.h"
#include "src/SwapChain.h"
#include "src/Buffer.h"
#include "src/Model.h"
#include "src/utils.h"
#include "src/Texture.h"
#include "src/descriptors/DescriptorSetLayout.h"
#include "src/Camera.h"
#include "src/CameraMovementController.h"
#include "src/RenderSystem.h"
#include "src/DrawableObject.h"
#include "src/VulkanApp.h"
#include "src/InstancedObjects.h"

class DefaultApp: public vtt::VulkanApp {
public:
    DefaultApp(int width, int height, const std::string &appName): VulkanApp(width, height, appName) {}

private:
    const std::string modelPath = "../models/viking_room.obj";
    const std::string texturePath = "../textures/viking_room.png";
    const vtt::RenderSystem::ShaderPaths vikingShaderPaths = vtt::RenderSystem::ShaderPaths {
            "../shaders/viking_room.vert.spv",
            "../shaders/viking_room.frag.spv"
    };

    const std::string axisModelPath = "../models/axis.obj";
    const std::string planeModelPath = "../models/quad.obj";
    const vtt::RenderSystem::ShaderPaths shaderPaths = vtt::RenderSystem::ShaderPaths {
            "../shaders/default.vert.spv",
            "../shaders/default.frag.spv"
    };

    const std::string sphereModelPath = "../models/sphere.obj";

    struct UniformBufferObject {
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3(1.0f, 1.0f, 0.0f));
    };

    struct InstanceData {
        glm::vec3 position;
        glm::vec3 color;
        float scale;
    };

    vtt::DrawableObject vikingRoom{vtt::Model::createModelFromFile(device, modelPath), std::make_shared<vtt::Texture>(device, texturePath)};
    vtt::DrawableObject axis{vtt::Model::createModelFromFile(device, axisModelPath)};
    vtt::DrawableObject plane{vtt::Model::createModelFromFile(device, planeModelPath)};
    vtt::DrawableObject sphere{vtt::Model::createModelFromFile(device, sphereModelPath)};


    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;

    vtt::RenderSystem roomSystem{device};
    std::vector<VkDescriptorSet> roomDescriptorSets;
    vtt::RenderSystem defaultSystem{device};
    std::vector<VkDescriptorSet> defaultDescriptorSets;

    vtt::Camera camera{};
    vtt::CameraMovementController cameraController{};


    float scale = 1, speed = 0, sphereSpeed = 0, damping = 0.05f; // sphereRadius = 0.641f;
    int rotAxis = 0;
    bool resetPos = false;

    void onCreate() override;

    void initializeObjects();

    void createUniformBuffers();

    void mainLoop(float deltaTime) override;

    void updateSphere(float deltaTime);

    void updateUniformBuffer(uint32_t frameIndex, float deltaTime);

    void showImGui();

};


#endif //VULKANTUTORIAL_DEFAULTAPP_H
