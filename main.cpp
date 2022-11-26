#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

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

#define INSTANCE_COUNT 1
const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 700;
const std::string APP_NAME = "Vulkan";

// TODO instancing using second vertex buffer
class TestApp: public vtt::VulkanApp {
public:
    TestApp(int width, int height, const std::string &appName): VulkanApp(width, height, appName) {}

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
        float scale;
    };

    vtt::DrawableObject vikingRoom{vtt::Model::createModelFromFile(device, modelPath), std::make_shared<vtt::Texture>(device, texturePath)};
    vtt::DrawableObject axis{vtt::Model::createModelFromFile(device, axisModelPath)};
    vtt::DrawableObject sphere{vtt::Model::createModelFromFile(device, sphereModelPath)};
    vtt::DrawableObject plane{vtt::Model::createModelFromFile(device, planeModelPath)};

    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;

    vtt::RenderSystem roomSystem{device};
    std::vector<VkDescriptorSet> roomDescriptorSets;
    vtt::RenderSystem defaultSystem{device};
    std::vector<VkDescriptorSet> defaultDescriptorSets;
    vtt::RenderSystem instanceSystem{device};

    vtt::Camera camera{};
    vtt::CameraMovementController cameraController{};

    std::vector<InstanceData> instancesData{INSTANCE_COUNT};
    std::unique_ptr<vtt::Buffer> instanceBuffer{};

    float scale = 1, speed = 0, sphereSpeed = 0, damping = 0.05f; // sphereRadius = 0.641f;
    int rotAxis = 0;
    bool resetPos = false;

    void onCreate() override {
        initializeObjects();

        createUniformBuffers();

        // Room render system
        {
            auto roomDescriptorLayout = vtt::DescriptorSetLayout::Builder(device)
                    .addBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr})
                    .addBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                 VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}).build();
            roomDescriptorSets = createDescriptorSets(roomDescriptorLayout,
                                                      {uniformBuffers[0]->descriptorInfo()},{vikingRoom.textureInfo()});
            roomSystem.createPipelineLayout(roomDescriptorLayout.descriptorSetLayout(), sizeof(vtt::DrawableObject::PushConstantData));
            roomSystem.createPipeline(renderer.renderPass(), vikingShaderPaths);
        }

        // Default render system
        auto defaultDescriptorLayout = vtt::DescriptorSetLayout::Builder(device)
                .addBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_ALL_GRAPHICS, nullptr})
                .build();
        defaultDescriptorSets = createDescriptorSets(defaultDescriptorLayout,{uniformBuffers[0]->descriptorInfo()});
        {
            defaultSystem.createPipelineLayout(defaultDescriptorLayout.descriptorSetLayout(), sizeof(vtt::DrawableObject::PushConstantData));
            defaultSystem.createPipeline(renderer.renderPass(), shaderPaths);
        }

        {
            instanceSystem.createPipelineLayout(defaultDescriptorLayout.descriptorSetLayout(), 0);
            instanceSystem.createPipeline(renderer.renderPass(), instanceShaderPaths);
        }
    }

    void initializeObjects() {
        camera.setViewTarget({0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 0.0f }, {0.0f, 1.0f, 0.0f});
        camera.m_rotation = {0, glm::radians(180.0f), glm::radians(180.0f)};

        vikingRoom.translate({-2.0f, -1.2f, -0.5f});
        vikingRoom.rotateAxis(2, glm::radians(-90.0f));
        vikingRoom.rotateAxis(1, glm::radians(-90.0f));

        axis.translate({3.0f, -1.2f, -1.8f});
        axis.setScale(0.125f);

        plane.translate({0.0f, -1.0f, 0.0f});

        sphere.setScale(0.01f);
    }

    void createInstances() {
        for (auto& instance: instancesData) {
            instance.position = glm::vec3(0.0f, 0.0f, 0.0f);
            instance.scale = 0.01f;
        }
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        uniformBuffers.resize(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < vtt::SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            uniformBuffers[i] = std::make_unique<vtt::Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void mainLoop(float deltaTime) override {

        cameraController.moveCamera(window.window(), deltaTime, camera);
        updateSphere(deltaTime);
        updateUniformBuffer(renderer.currentFrame(), deltaTime);

        renderer.runFrame([&](VkCommandBuffer commandBuffer){
            showImGui();

            renderer.runRenderPass([&](VkCommandBuffer& commandBuffer){
                roomSystem.bind(commandBuffer, &roomDescriptorSets[renderer.currentFrame()]);
                sphere.render(roomSystem, commandBuffer);
                vikingRoom.render(roomSystem, commandBuffer);

                defaultSystem.bind(commandBuffer, &defaultDescriptorSets[renderer.currentFrame()]);
                axis.render(defaultSystem, commandBuffer);
                plane.render(defaultSystem, commandBuffer);
            });
        });

    }

    void updateSphere(float deltaTime){
        float a = 5;
        sphere.translate(deltaTime * sphereSpeed * glm::vec3(0.0f, 1.0f, 0.0f));
        sphereSpeed -= a*deltaTime;
        if (sphere.m_translation.y < plane.m_translation.y) {
            sphere.m_translation.y = plane.m_translation.y;
            sphereSpeed *= (damping - 1);
        }

    }

    void updateUniformBuffer(uint32_t frameIndex, float deltaTime){
        if (resetPos){
            for (int i = 0; i < 3; i++){
                if (i != rotAxis) vikingRoom.resetRotation(i);
            }
        }
        vikingRoom.setScale(scale);
        vikingRoom.rotateAxis(rotAxis, speed * deltaTime * glm::radians(90.0f));

        UniformBufferObject ubo{};
        camera.setPerspectiveProjection(glm::radians(50.f), renderer.getSwapChainAspectRatio(), 0.1f, 100.f);
        ubo.view = camera.getView();
        ubo.proj = camera.getProjection();
        uniformBuffers[frameIndex]->singleWrite(&ubo);

    }

    void showImGui(){
        static bool show_demo_window = false, show_another_window = false;

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // main m_windowRef
        {

            ImGui::Begin("Control Panel");

            ImGui::Text("Other windows");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::CollapsingHeader("Viking Room")) {

                ImGui::Text("Choose rotation axis:");
                ImGui::RadioButton("X-axis", &rotAxis, 2); ImGui::SameLine();
                ImGui::RadioButton("Y-axis", &rotAxis, 1); ImGui::SameLine();
                ImGui::RadioButton("Z-axis", &rotAxis, 0);
                ImGui::Checkbox("Reset Position", &resetPos);
                ImGui::SliderFloat("scale", &scale, 0.1f, 4.0f);
                ImGui::SliderFloat("speed", &speed, -4.0f, 4.0f);
//            static auto color = glm::vec3(0.0f);
//            ImGui::ColorEdit3("clear color", (float *) &color);

                if (ImGui::Button("Reset speed")) speed = 0;
            }

            if (ImGui::CollapsingHeader("Plane")) {

                ImGui::SliderFloat("x", &plane.m_translation.x, -2.0f, 2.0f);
                ImGui::SliderFloat("y", &plane.m_translation.y, -4.0f, 0.0f);
                ImGui::SliderFloat("z", &plane.m_translation.z, -2.0f, 2.0f);

//            static auto color = glm::vec3(0.0f);
//            ImGui::ColorEdit3("clear color", (float *) &color);

            }

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple m_windowRef.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the m_windowRef will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another m_windowRef!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

    }

};

int main() {
    TestApp app{WIDTH, HEIGHT, APP_NAME};

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
