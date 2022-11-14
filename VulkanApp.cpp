//
// Created by luc on 14/11/22.
//

#include "VulkanApp.h"

namespace vtt {

    VulkanApp::VulkanApp(int width, int height, const std::string &appName): window(width, height, appName), m_appName(appName) {
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

        renderer.activateImGui(descriptorPool->descriptorPool());

    }

    void VulkanApp::run() {
        {
            onCreate();

            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime, accTime = 0;
            uint32_t frames = 0;

            while (!window.shouldClose()) {
                auto newTime = std::chrono::high_resolution_clock::now();
                deltaTime =
                        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
                currentTime = newTime;
                accTime += deltaTime;
                frames++;

                glfwPollEvents();
                mainLoop(deltaTime);

                if (accTime > 0.5f) {
                    double fps = double(frames) / accTime;

                    std::stringstream ss;
                    ss << m_appName <<" " << "[" << fps << " FPS]";

                    glfwSetWindowTitle(window.window(), ss.str().c_str());

                    frames = 0;
                    accTime = 0.0f;
                }
            }
            vkDeviceWaitIdle(device.device());

            onDestroy();

        }
    }
}