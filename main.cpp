#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define STB_IMAGE_IMPLEMENTATION

#include <sstream>
#include <fstream>
#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_glfw.h"
#include "external/imgui/imgui_impl_vulkan.h"
#include "external/stbimage/stb_image.h"
#include "external/objloader/tiny_obj_loader.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Image.h"
#include "Model.h"
#include "utils.h"

const uint32_t WIDTH = 1000;
const uint32_t HEIGHT = 700;


#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

static std::vector<char> readFile(const std::string& filename){
    std::ifstream file;
    file.open(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file: " + filename);
    }

    long fileSize = (long) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}



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

    // config
    vtt::Window window{WIDTH, HEIGHT, "Vulkan"};
    vtt::Device device{window};

    // update
    std::unique_ptr<vtt::SwapChain> swapChain;

    VkDescriptorSetLayout descriptorSetLayout{}; // config

    // config
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipelineFill{};
    VkPipeline graphicsPipelineLine{};

    std::unique_ptr<vtt::Model> model;

    std::vector<std::unique_ptr<vtt::Buffer>> uniformBuffers;

    // config
    std::vector<VkCommandBuffer> commandBuffers{};

    uint32_t texMipLevels{};
    std::unique_ptr<vtt::Image> textureImage;
    VkSampler textureSampler{};


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


        if (lineMode) vkCmdBindPipeline(curCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineLine);
        else vkCmdBindPipeline(curCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineFill);

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

        vkDestroySampler(device.device(), textureSampler, nullptr);


        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);

        vkDestroyPipeline(device.device(), graphicsPipelineFill, nullptr);
        vkDestroyPipeline(device.device(), graphicsPipelineLine, nullptr);
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();
    }

    void initVulkan() {
        swapChain = std::make_unique<vtt::SwapChain>(device, window.extent());
        createDescriptorSetLayout();
        // pipeline
        createGraphicsPipeline();
        // model
        model = vtt::Model::createModelFromFile(device, modelPath);
        createTextureImage();
        createTextureSampler();
        createUniformBuffers();
        // descriptor
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffer();
        // control
        initImGuiVulkan();
    }

    void createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device.physicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(texMipLevels);

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels)
            throw std::runtime_error("failed to load texture!");

        texMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        vtt::Buffer stagingBuffer(device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.singleWrite(pixels);

        stbi_image_free(pixels);

        textureImage = std::make_unique<vtt::Image>(device, texWidth, texHeight, texMipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT ,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
        textureImage->transitionLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, texMipLevels);
        device.copyBufferToImage(stagingBuffer.get(), textureImage->image(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        // transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
        generateMipMaps(textureImage->image(), VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, texMipLevels);

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
            imageInfo.imageView = textureImage->view();
            imageInfo.sampler = textureSampler;

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

    void createGraphicsPipeline(){
        auto vertShaderCode = readFile("../shaders/vert.spv");
        auto fragShaderCode = readFile("../shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{};
        vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCreateInfo.module = vertShaderModule;
        vertShaderStageCreateInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageInfo};

        auto bindingDescription = vtt::Model::Vertex::getBindingDescription();
        auto attributeDescription = vtt::Model::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.rasterizationSamples = device.msaaSamples();
        multisampling.sampleShadingEnable = VK_FALSE; // VK_TRUE for more image quality
        multisampling.minSampleShading = 1.0f; // 0.2f for more image quality
        multisampling.pSampleMask = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable = VK_FALSE; // Optional

        VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.depthTestEnable = VK_TRUE;
        depthStencilInfo.depthWriteEnable = VK_TRUE;
        depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilInfo.minDepthBounds = 0.0f; // Optional
        depthStencilInfo.maxDepthBounds = 1.0f; // Optional
        depthStencilInfo.stencilTestEnable = VK_FALSE;
        depthStencilInfo.front = {}; // Optional
        depthStencilInfo.back = {}; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencilInfo;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = swapChain->renderPass();
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineFill) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

        if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipelineLine) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device.device(), vertShaderModule, nullptr);
        vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);

    }
    
    VkShaderModule createShaderModule(const std::vector<char>& code){
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS){
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
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

    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
        VkFormatProperties formatProperties{};
        vkGetPhysicalDeviceFormatProperties(device.physicalDevice(), imageFormat, &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        device.executeSingleCommand([image, texWidth, texHeight, mipLevels](VkCommandBuffer commandBuffer) {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = image;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.layerCount = 1;

            int32_t mipWidth = texWidth;
            int32_t mipHeight = texHeight;

            for (uint32_t i = 1; i < mipLevels; ++i) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &barrier);

                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;

                vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1, &blit, VK_FILTER_LINEAR);

                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(commandBuffer,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &barrier);

                if (mipWidth > 1) mipWidth /= 2;
                if (mipHeight > 1) mipHeight /= 2;
            }

            barrier.subresourceRange.baseMipLevel = mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                                 0, nullptr,
                                 0, nullptr,
                                 1, &barrier);

        });


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
