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
#include "DescriptorSetLayout.h"

const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 700;

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

bool forward = false, backward = false, up = false, down = false, left = false, right = false;

class HelloTriangleApplication {
public:

    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const std::string modelPath = "../models/viking_room.obj";
    const std::string texturePath = "../textures/viking_room.png";
    const std::string fragPath = "../shaders/frag.spv";
    const std::string vertPath = "../shaders/vert.spv";

    // config
    vtt::Window window{WIDTH, HEIGHT, "Vulkan"};
    vtt::Device device{window};
    // update
    VkDescriptorPool descriptorPool{};

    vtt::Renderer renderer{window, device};
    std::unique_ptr<vtt::DescriptorSetLayout> descriptorLayout;
    // config
    VkPipelineLayout pipelineLayout{};

    std::unique_ptr<vtt::Pipeline> pipelineFill;
    std::unique_ptr<vtt::Pipeline> pipelineLine;

    std::unique_ptr<vtt::Model> model = vtt::Model::createModelFromFile(device, modelPath);
    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;
    vtt::Texture texture{device, texturePath};

    // update
    std::vector<VkDescriptorSet> descriptorSets;

    // camera
    glm::vec3 pos{2.0f, 2.0f, 2.0f};
    glm::vec3 dir{-1.0f, -1.0f, -1.0f};
    glm::vec3 cameraPlane{-1.0f, 0.0f, 1.0f};

    // fps
    double startTime{};
    uint32_t frames = 0;

    //imgui stuff
    float scale = 1, speed = 0;
    int axis = 0;
    bool resetPos = true;
    bool lineMode = false;

    // movement
    float cameraSpeed = 1e-2;
    float angleSpeed = 1e-3;

    void mainLoop() {
        startTime = glfwGetTime();
        while (!window.shouldClose()) {
            glfwPollEvents();
            glfwSetKeyCallback(window.window(), keyCallback);
            handleInputs();
            drawFrame();
            showFps();
        }

        vkDeviceWaitIdle(device.device());
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
        if (action == GLFW_PRESS)
            switch (key) {
                case GLFW_KEY_W: forward = true; break;
                case GLFW_KEY_S: backward = true; break;
                case GLFW_KEY_Q: up = true; break;
                case GLFW_KEY_E: down = true; break;
                case GLFW_KEY_A: left = true; break;
                case GLFW_KEY_D: right = true; break;
                default: break;
            }
        else if (action == GLFW_RELEASE)
            switch (key) {
                case GLFW_KEY_W: forward = false; break;
                case GLFW_KEY_S: backward = false; break;
                case GLFW_KEY_Q: up = false; break;
                case GLFW_KEY_E: down = false; break;
                case GLFW_KEY_A: left = false; break;
                case GLFW_KEY_D: right = false; break;
                default: break;
            }
    }

    void handleInputs(){
        if (glm::dot(dir, dir) - 1 > 0.00001) dir = glm::normalize(dir);
        if (forward) {
            pos += cameraSpeed*dir;
        } else if (backward) {
            pos -= cameraSpeed*dir;
        }
        if (up) {
            auto otherPlane = glm::cross(dir, cameraPlane);
            dir = glm::rotate(glm::mat4(1.0f), angleSpeed, otherPlane) * glm::vec4(dir, 0);
            cameraPlane = glm::rotate(glm::mat4(1.0f), angleSpeed, otherPlane) * glm::vec4(cameraPlane, 0);
        } else if (down) {
            auto otherPlane = glm::cross(dir, cameraPlane);
            dir = glm::rotate(glm::mat4(1.0f), -angleSpeed, otherPlane) * glm::vec4(dir, 0);
            cameraPlane = glm::rotate(glm::mat4(1.0f), -angleSpeed, otherPlane) * glm::vec4(cameraPlane, 0);
        }
        if (left) {
            dir = glm::rotate(glm::mat4(1.0f), angleSpeed, cameraPlane) * glm::vec4(dir, 0);
//            cameraPlane = glm::rotate(glm::mat4(1.0f), angleSpeed, otherPlane) * glm::vec4(cameraPlane, 0);
        } else if (right) {
            dir = glm::rotate(glm::mat4(1.0f), -angleSpeed, cameraPlane) * glm::vec4(dir, 0);
//            cameraPlane = glm::rotate(glm::mat4(1.0f), -angleSpeed, otherPlane) * glm::vec4(cameraPlane, 0);
        }


    }

    void showFps(){
        double currentTime = glfwGetTime();
        double elapsedTime = currentTime - startTime;
        frames++;
        if (elapsedTime > 0.5){

            double fps = double(frames) / elapsedTime;

            std::stringstream ss;
            ss << "Vulkan " << "[" << fps << " FPS]";

            glfwSetWindowTitle(window.window(), ss.str().c_str());

            frames = 0;
            startTime = currentTime;
        }

    }

    void drawFrame(){
        updateUniformBuffer(renderer.currentFrame());

        renderer.runFrame([this](VkCommandBuffer commandBuffer){
            showImGui();
            recordCommandBuffer();
        });

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

    void recordCommandBuffer(){
        renderer.runRenderPass([this](VkCommandBuffer& commandBuffer){
            if (lineMode) pipelineLine->bind(commandBuffer);
            else pipelineFill->bind(commandBuffer);

            model->bind(commandBuffer);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                    0, 1, &descriptorSets[renderer.currentFrame()], 0, nullptr);
            model->draw(commandBuffer);

        });

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
        ubo.view = glm::lookAt(pos, pos + dir,cameraPlane);
        ubo.proj = glm::perspective(glm::radians(45.0f), renderer.getSwapChainAspectRatio(),
                                    0.1f, 100.0f);

        ubo.proj[1][1] *= -1;

        uniformBuffers[currentImage]->singleWrite(&ubo);
        startTime = std::chrono::high_resolution_clock::now();
    }

    void cleanup() {
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);

        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);


    }

    void initVulkan() {
        createDescriptorSetLayout();
        createUniformBuffers();
        createGraphicsPipeline();
        // descriptor
        createDescriptorPool();
        renderer.activateImGui(descriptorPool);
        createDescriptorSets();
        // control

    }

    void createGraphicsPipeline(){
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{descriptorLayout->descriptorSetLayout()};
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        auto pipelineConfigInfo = vtt::Pipeline::defaultConfigInfo(pipelineLayout, renderer.renderPass());

        pipelineFill = std::make_unique<vtt::Pipeline>(device, "../shaders/vert.spv",
                                                   "../shaders/frag.spv", pipelineConfigInfo);

        pipelineConfigInfo.enablePolygonLineMode();
        pipelineLine = std::make_unique<vtt::Pipeline>(device, "../shaders/vert.spv",
                                                       "../shaders/frag.spv", pipelineConfigInfo);

    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorLayout->descriptorSetLayout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device.device(), &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < vtt::SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i]->get();
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture.view();
            imageInfo.sampler = texture.sampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

    }

    void createDescriptorPool() {

        std::array<VkDescriptorPoolSize, 11> poolSizes = {{
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        }};

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1000 * poolSizes.size();
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        if (vkCreateDescriptorPool(device.device(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS){
            throw std::runtime_error("failed to create descriptor pool!");
        }


//        std::array<VkDescriptorPoolSize, 2> poolSizes{};
//        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        poolSizes[0].descriptorCount = static_cast<uint32_t>(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
//        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//        poolSizes[1].descriptorCount = static_cast<uint32_t>(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
//
//        VkDescriptorPoolCreateInfo poolInfo{};
//        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//        poolInfo.pPoolSizes = poolSizes.data();
//        poolInfo.maxSets = static_cast<uint32_t>(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);
//
//        if (vkCreateDescriptorPool(m_deviceRef, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create descriptor pool!");
//        }


    }

    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < vtt::SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            uniformBuffers[i] = std::make_unique<vtt::Buffer>(device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        }
    }

    void createDescriptorSetLayout(){
        descriptorLayout = vtt::DescriptorSetLayout::Builder(device)
                .addBinding({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr})
                .addBinding({1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr})
                .build();
    }
};

int main() {

//    system("glslc ../shaders/shader.vert -o ../shaders/vert.spv");
//    system("glslc ../shaders/shader.frag -o ../shaders/frag.spv");
    HelloTriangleApplication app{};

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
