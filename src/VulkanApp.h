//
// Created by luc on 14/11/22.
//

#ifndef VULKANTUTORIAL_VULKANAPP_H
#define VULKANTUTORIAL_VULKANAPP_H

#include <sstream>
#include <fstream>
#include "../external/imgui/imgui.h"
#include "../external/objloader/tiny_obj_loader.h"
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

namespace vtt{
    class VulkanApp {
    public:
        VulkanApp(int width, int height, const std::string &appName);
        VulkanApp(const VulkanApp &) = delete;
        VulkanApp &operator=(const VulkanApp &) = delete;
        ~VulkanApp() = default;

        void run();

    private:
        std::string m_appName;

    protected:
        virtual void mainLoop(float deltaTime) = 0;
        virtual void onCreate() = 0;
        virtual void onDestroy() {};

        vtt::Window window;
        vtt::Device device{window};
        std::unique_ptr<vtt::DescriptorPool> descriptorPool;

        vtt::Renderer renderer{window, device};

    };
}



#endif //VULKANTUTORIAL_VULKANAPP_H
