//
// Created by luc on 13/12/22.
//

#ifndef VULKANTUTORIAL_INSTANCINGAPP_H
#define VULKANTUTORIAL_INSTANCINGAPP_H

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

class InstancingApp: public vtt::VulkanApp {
public:
    InstancingApp(int width, int height, const std::string &appName, vtt::Device::PhysicalDeviceType type = vtt::Device::INTEL):
    VulkanApp(width, height, appName, type) {}

private:
    uint32_t INSTANCE_COUNT = 65536;

    const std::string planeModelPath = "../models/quad.obj";
    const vtt::RenderSystem::ShaderPaths shaderPaths = vtt::RenderSystem::ShaderPaths {
            "../shaders/default.vert.spv",
            "../shaders/default.frag.spv"
    };

    const std::string sphereModelPath = "../models/lowsphere.obj";
    const vtt::RenderSystem::ShaderPaths instanceShaderPaths = vtt::RenderSystem::ShaderPaths {
            "../shaders/instancing.vert.spv",
            "../shaders/instancing.frag.spv"
    };

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

    vtt::DrawableObject plane{vtt::Model::createModelFromFile(device, planeModelPath)};

    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;

    vtt::RenderSystem defaultSystem{device};
    std::vector<VkDescriptorSet> defaultDescriptorSets;
    vtt::RenderSystem instanceSystem{device};

    vtt::Camera camera{};
    vtt::CameraMovementController cameraController{};

    vtt::InstancedObjects<InstanceData> instancedSpheres{device, INSTANCE_COUNT, vtt::Model::createModelFromFile(device, sphereModelPath)};

    float damping = 0.05f, sphereRadius = 0.641f;
    std::vector<float> sphereSpeeds;
    std::vector<uint32_t> iter;
    float gpuTime = 0, cpuTime = 0;
    bool activateTimer = false;

    void onCreate() override;
    void initializeObjects();
    void createInstances();
    void createUniformBuffers();
    void mainLoop(float deltaTime) override;
    void updateSpheres(float deltaTime);
    void updateUniformBuffer(uint32_t frameIndex, float deltaTime);
    void showImGui();

};


#endif //VULKANTUTORIAL_INSTANCINGAPP_H
