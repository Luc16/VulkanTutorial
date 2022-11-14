#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <sstream>
#include <fstream>
#include "external/imgui/imgui.h"
#include "external/objloader/tiny_obj_loader.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Model.h"
#include "utils.h"
#include "Texture.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "descriptors/DescriptorSetLayout.h"
#include "descriptors/DescriptorPool.h"
#include "descriptors/DescriptorWriter.h"
#include "Camera.h"
#include "CameraMovementController.h"
#include "RenderSystem.h"
#include "DrawableObject.h"

const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 700;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    // TODO ver exatamente o alinhamento certo
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3(-1.0f, 0.0f, 1.0f));
};

bool forward = false, backward = false, up = false, down = false, left = false, right = false;

class HelloTriangleApplication {
public:

    void run() {
        initVulkan();
        mainLoop();
    }

private:
    const std::string modelPath = "../models/viking_room.obj";
    const std::string texturePath = "../textures/viking_room.png";
    vtt::RenderSystem::ShaderPaths vikingShaderPaths = vtt::RenderSystem::ShaderPaths {
        "../shaders/viking_room.vert.spv",
        "../shaders/viking_room.frag.spv"
    };

    const std::string planeModelPath = "../models/quad.obj";
    vtt::RenderSystem::ShaderPaths shaderPaths = vtt::RenderSystem::ShaderPaths {
            "../shaders/vert.spv",
            "../shaders/frag.spv"
    };

    vtt::Window window{WIDTH, HEIGHT, "Vulkan"};
    vtt::Device device{window};
    std::unique_ptr<vtt::DescriptorPool> descriptorPool;

    vtt::Renderer renderer{window, device};

    vtt::DrawableObject vikingRoom{vtt::Model::createModelFromFile(device, modelPath), std::make_shared<vtt::Texture>(device, texturePath)};
    vtt::DrawableObject plane{vtt::Model::createModelFromFile(device, planeModelPath)};

    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;


    vtt::Camera camera{};
    vtt::CameraMovementController cameraController{};

    //imgui stuff
    float scale = 1, speed = 0;
    int axis = 0;
    bool resetPos = true;
    bool lineMode = false;

    void mainLoop() {

        auto currentTime = std::chrono::high_resolution_clock::now();
        float frameTime, accTime = 0;
        uint32_t frames = 0;

        camera.setViewDirection({2, 2, 2}, {-1, -1, -1});

        auto roomDescriptorLayout = vtt::DescriptorSetLayout::Builder(device)
                .addBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr})
                .addBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr})
                .build();
        auto defaultDescriptorLayout = vtt::DescriptorSetLayout::Builder(device)
                .addBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr})
                .build();
        std::vector<VkDescriptorSet> roomDescriptorSets(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
        VkDescriptorImageInfo imageInfo = vikingRoom.textureInfo();

        std::vector<VkDescriptorSet> defaultDescriptorSets(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < vtt::SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            VkDescriptorBufferInfo bufferInfo = uniformBuffers[i]->descriptorInfo();

            vtt::DescriptorWriter(*defaultDescriptorLayout, *descriptorPool)
                    .writeBuffer(0, &bufferInfo)
                    .build(defaultDescriptorSets[i]);

            vtt::DescriptorWriter(*roomDescriptorLayout, *descriptorPool)
                    .writeBuffer(0, &bufferInfo)
                    .writeImage(1, &imageInfo)
                    .build(roomDescriptorSets[i]);
        }

        vtt::RenderSystem roomSystem(device, renderer.renderPass(), roomDescriptorLayout->descriptorSetLayout(), vikingShaderPaths);
        vtt::RenderSystem defaultSystem(device, renderer.renderPass(), defaultDescriptorLayout->descriptorSetLayout(), shaderPaths);

        while (!window.shouldClose()) {
            glfwPollEvents();

            // update fps
            {
                auto newTime = std::chrono::high_resolution_clock::now();
                frameTime =
                        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
                currentTime = newTime;
                accTime += frameTime;
                frames++;
            }

//            cameraController.moveCamera(window.window(), frameTime, camera);
            updateUniformBuffer(renderer.currentFrame());

            renderer.runFrame([&](VkCommandBuffer commandBuffer){
                showImGui();

                renderer.runRenderPass([&](VkCommandBuffer& commandBuffer){
                    roomSystem.bind(commandBuffer, &roomDescriptorSets[renderer.currentFrame()]);
                    vikingRoom.render(commandBuffer);

                    defaultSystem.bind(commandBuffer, &defaultDescriptorSets[renderer.currentFrame()]);
                    plane.render(commandBuffer);
                });
            });

            // show fps
            if (accTime > 0.5f) {
                double fps = double(frames) / accTime;

                std::stringstream ss;
                ss << "Vulkan " << "[" << fps << " FPS]";

                glfwSetWindowTitle(window.window(), ss.str().c_str());

                frames = 0;
                accTime = 0;
            }
        }

        vkDeviceWaitIdle(device.device());
    }

    void showImGui(){
        static bool show_demo_window = false, show_another_window = false;

        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // main m_windowRef
        {
            static int counter = 0;

            ImGui::Begin("Control Panel");

            ImGui::Text("Other windows");
            ImGui::Checkbox("Demo Window", &show_demo_window);
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("Activate line polygon mode", &lineMode);
            ImGui::Text("Choose rotation axis:");
            ImGui::RadioButton("X-axis", &axis, 2); ImGui::SameLine();
            ImGui::RadioButton("Y-axis", &axis, 1); ImGui::SameLine();
            ImGui::RadioButton("Z-axis", &axis, 0);
            ImGui::Checkbox("Reset Position", &resetPos);
            ImGui::SliderFloat("scale", &scale, 0.1f, 4.0f);
            ImGui::SliderFloat("speed", &speed, -4.0f, 4.0f);
//            static auto color = glm::vec3(0.0f);
//            ImGui::ColorEdit3("clear color", (float *) &color);

            if (ImGui::Button("Reset speed")) speed = 0;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

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

    void updateUniformBuffer(uint32_t currentImage){
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentTime-startTime).count();
        static std::array<float, 3> angles = {0, 0, 0};
        if (resetPos){
            for (int i = 0; i < angles.size(); ++i) {
                if (i != axis) angles[i] = 0;
            }
        }
        angles[axis] += speed * deltaTime * glm::radians(90.0f);

        UniformBufferObject ubo{};
        ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(scale))
                * glm::rotate(glm::mat4(1.0f), angles[0],glm::vec3(0.0f, 0.0f, 1.0f))
                * glm::rotate(glm::mat4(1.0f), angles[1],glm::vec3(0.0f, 1.0f, 0.0f))
                * glm::rotate(glm::mat4(1.0f), angles[2],glm::vec3(1.0f, 0.0f, 0.0f));

        camera.setPerspectiveProjection(glm::radians(50.f), renderer.getSwapChainAspectRatio(), 0.1f, 100.f);
        ubo.view = camera.getView();
        ubo.proj = camera.getProjection();

        uniformBuffers[currentImage]->singleWrite(&ubo);
        startTime = std::chrono::high_resolution_clock::now();
    }

    void initVulkan() {
        createUniformBuffers();
        createDescriptors();

        renderer.activateImGui(descriptorPool->descriptorPool());
    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < vtt::SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            uniformBuffers[i] = std::make_unique<vtt::Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void createDescriptors(){

        descriptorPool = vtt::DescriptorPool::Builder(device)
                .addPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 })
                .addPoolSize({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 })
                .setFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                .setMaxSetsTimesSizes(1000)
                .build();
    }
};

int main() {

    system("glslc ../shaders/shader.vert -o ../shaders/vert.spv");
    system("glslc ../shaders/shader.frag -o ../shaders/frag.spv");
    HelloTriangleApplication app{};

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
