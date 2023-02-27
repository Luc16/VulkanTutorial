//
// Created by luc on 13/12/22.
//

#ifndef VULKANTUTORIAL_DEFAULTAPP_H
#define VULKANTUTORIAL_DEFAULTAPP_H

#include <sstream>
#include "external/imgui/imgui.h"
#include "external/objloader/tiny_obj_loader.h"
#include "src/lib/SwapChain.h"
#include "src/lib/Buffer.h"
#include "src/lib/Model.h"
#include "src/lib/utils.h"
#include "src/lib/Texture.h"
#include "src/lib/descriptors/DescriptorSetLayout.h"
#include "src/lib/Camera.h"
#include "src/lib/CameraMovementController.h"
#include "src/lib/RenderSystem.h"
#include "src/lib/DrawableObject.h"
#include "src/lib/VulkanApp.h"
#include "src/lib/InstancedObjects.h"

class DefaultApp: public vkb::VulkanApp {
public:
    DefaultApp(int width, int height, const std::string &appName, vkb::Device::PhysicalDeviceType type = vkb::Device::INTEL):
            VulkanApp(width, height, appName, type) {}

private:
    const std::string modelPath = "../models/viking_room.obj";
    const std::string texturePath = "../textures/viking_room.png";
    const vkb::RenderSystem::ShaderPaths vikingShaderPaths = vkb::RenderSystem::ShaderPaths {
            "../shaders/viking_room.vert.spv",
            "../shaders/viking_room.frag.spv"
    };

    const std::string axisModelPath = "../models/axis.obj";
    const std::string planeModelPath = "../models/quad.obj";
    const vkb::RenderSystem::ShaderPaths shaderPaths = vkb::RenderSystem::ShaderPaths {
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

    vkb::DrawableObject vikingRoom{vkb::Model::createModelFromFile(device, modelPath), std::make_shared<vkb::Texture>(device, texturePath)};
    vkb::DrawableObject axis{vkb::Model::createModelFromFile(device, axisModelPath)};
    vkb::DrawableObject plane{vkb::Model::createModelFromFile(device, planeModelPath)};
    vkb::DrawableObject sphere{vkb::Model::createModelFromFile(device, sphereModelPath)};


    std::vector<std::unique_ptr<vkb::Buffer>> uniformBuffers;

    vkb::RenderSystem roomSystem{device};
    std::vector<VkDescriptorSet> roomDescriptorSets;
    vkb::RenderSystem defaultSystem{device};
    std::vector<VkDescriptorSet> defaultDescriptorSets;

    vkb::Camera camera{};
    vkb::CameraMovementController cameraController{};


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
