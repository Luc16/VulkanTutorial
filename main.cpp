#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <sstream>
#include <fstream>
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "external/objloader/tiny_obj_loader.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Image.h"
#include "Model.h"
#include "utils.h"
#include "Texture.h"
#include "Pipeline.h"

const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 700;


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif


struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

//const std::vector<Vertex> vertices = {
//        {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
//        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
//
//        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
//        {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
//        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
//        {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
//
//        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
//        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//        {{0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
//
//        {{-0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
//        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
//        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//        {{0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
//
//        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//        {{0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
//        {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//
//        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
//        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
//        {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
//        {{-0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
//};
//
//const std::vector<uint16_t> indices = {
//        0, 1, 2, 2, 3, 0,
//        4, 5, 6, 6, 7, 4,
//        8, 9, 10, 10, 11, 8,
//        12, 13, 14, 14, 15, 12,
//        16, 17, 18, 18, 19, 16,
//        20, 21, 22, 22, 23, 20,
//};

bool forward = false, backward = false, up = false, down = false, left = false, right = false;

class HelloTriangleApplication {
public:

    void run() {
        initWindow();
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
    std::unique_ptr<vtt::SwapChain> swapChain = std::make_unique<vtt::SwapChain>(device, window.extent());

    VkDescriptorSetLayout descriptorSetLayout{}; // config

    // config
    VkPipelineLayout pipelineLayout{};
    std::unique_ptr<vtt::Pipeline> pipeline;

    std::unique_ptr<vtt::Model> model = vtt::Model::createModelFromFile(device, modelPath);

    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;

    vtt::Texture texture{device, texturePath};
    // config
    std::vector<VkCommandBuffer> commandBuffers{};



    VkDescriptorPool descriptorPool{};
    // update
    std::vector<VkDescriptorSet> descriptorSets;

    // camera
    glm::vec3 pos{2.0f, 2.0f, 2.0f};
    glm::vec3 dir{-1.0f, -1.0f, -1.0f};
    glm::vec3 cameraPlane{-1.0f, 0.0f, 1.0f};

    // fps
    double startTime{};
    uint32_t frames = 0;
    uint32_t currentFrame = 0;

    //imgui stuff
    float scale = 1, speed = 0;
    int axis = 0;
    bool resetPos = true;
    bool lineMode = false;

    // movement
    float cameraSpeed = 1e-2;
    float angleSpeed = 1e-3;

    void initWindow() {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        ImGui_ImplGlfw_InitForVulkan(window.window(), true);
        ImGui::StyleColorsDark();

    }

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

        uint32_t imageIndex;
        VkResult result = swapChain->acquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapChain();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        showImGui();

        updateUniformBuffer(currentFrame);

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);

        ImGui::Render();
        auto imGuiDrawData = ImGui::GetDrawData();
        recordCommandBuffer(commandBuffers[currentFrame], imageIndex, imGuiDrawData);

        result = swapChain->submitCommandBuffers(&commandBuffers[currentFrame], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.frameBufferResized()) {
            window.frameBufferNotResized();
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % vtt::SwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void showImGui(){
        static bool show_demo_window = false, show_another_window = false;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

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

    void recordCommandBuffer(VkCommandBuffer curCommandBuffer, uint32_t imageIndex, ImDrawData* drawData){
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(curCommandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command m_buffer!");
        }

        swapChain->beginRenderPass(curCommandBuffer, imageIndex);

        if (lineMode) pipeline->bind(curCommandBuffer);
        else pipeline->bind(curCommandBuffer);

        model->bind(curCommandBuffer);

        vkCmdBindDescriptorSets(curCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                                0, 1, &descriptorSets[currentFrame], 0, nullptr);
        model->draw(curCommandBuffer);

        ImGui_ImplVulkan_RenderDrawData(drawData, curCommandBuffer);

        swapChain->endRenderPass(curCommandBuffer);

        if (vkEndCommandBuffer(curCommandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command m_buffer!");
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
        ubo.view = glm::lookAt(pos, pos + dir,cameraPlane);
        ubo.proj = glm::perspective(glm::radians(45.0f), (float) swapChain->width() / (float) swapChain->height(),
                                    0.1f, 100.0f);

        ubo.proj[1][1] *= -1;

        uniformBuffers[currentImage]->singleWrite(&ubo);
        startTime = std::chrono::high_resolution_clock::now();
    }

    void cleanup() {
        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);

        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }

    void initVulkan() {
        createDescriptorSetLayout();
        createUniformBuffers();

        createGraphicsPipeline();
        // descriptor
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffer();
        // control
        initImGuiVulkan();
    }

    void createGraphicsPipeline(){
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        vtt::Pipeline::PipelineConfigInfo pipelineConfigInfo = vtt::Pipeline::defaultConfigInfo(pipelineLayout, swapChain->renderPass());

        pipeline = std::make_unique<vtt::Pipeline>(device, "../shaders/vert.spv",
                                                   "../shaders/frag.spv", pipelineConfigInfo);

    }

    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    void initImGuiVulkan() {
        ImGui_ImplVulkan_InitInfo initInfo = {};
        initInfo.Instance = device.instance();
        initInfo.PhysicalDevice = device.physicalDevice();
        initInfo.Device = device.device();
        initInfo.QueueFamily = swapChain->queueFamily();
        initInfo.Queue = device.graphicsQueue();
        initInfo.PipelineCache = nullptr;
        initInfo.DescriptorPool = descriptorPool;
        initInfo.Subpass = 0;
        initInfo.MinImageCount = swapChain->imageCount();
        initInfo.ImageCount = static_cast<uint32_t>(swapChain->numImages());
        initInfo.MSAASamples = device.msaaSamples();
        initInfo.Allocator = nullptr;
        initInfo.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&initInfo, swapChain->renderPass());

        {
            VkResult err;
            // Use any command queue
            VkCommandPool command_pool = device.commandPool();
            VkCommandBuffer command_buffer = commandBuffers[0];

            err = vkResetCommandPool(device.device(), command_pool, 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;
            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);
            err = vkQueueSubmit(device.graphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(device.device());
            check_vk_result(err);
            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }

    void createDescriptorSets(){
        std::vector<VkDescriptorSetLayout> layouts(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
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
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        samplerLayoutBinding.pImmutableSamplers = nullptr;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create descriptor set layout!");
        }

    }

    void createCommandBuffer(){
        commandBuffers.resize(vtt::SwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = device.commandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();

        if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }

    }

    void recreateSwapChain(){
        int width = 0, height = 0;
        glfwGetFramebufferSize(window.window(), &width, &height);
        if (width == 0 && height == 0){
            glfwGetFramebufferSize(window.window(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());

        swapChain = std::make_unique<vtt::SwapChain>(device, window.extent());
        ImGui_ImplVulkan_SetMinImageCount(swapChain->imageCount());
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
