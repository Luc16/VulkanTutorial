//
// Created by luc on 26/10/22.
//

#ifndef VULKANTUTORIAL_DEVICE_H
#define VULKANTUTORIAL_DEVICE_H

#include <vulkan/vulkan.h>
#include <vector>
#include "utils.h"
#include "Window.h"
#include <GLFW/glfw3.h>

namespace vkb{
    class Device {
    public:
#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif
        static constexpr VkSampleCountFlagBits MAX_SAMPLES = VK_SAMPLE_COUNT_4_BIT;

        enum PhysicalDeviceType {
            NVIDIA = 8089,
            INTEL = 39876
        };

        struct QueueFamilyIndices {
            std::optional<uint32_t> graphicsFamily{};
            std::optional<uint32_t> presentFamily{};


            [[nodiscard]] bool isComplete() const {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities{};
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        explicit Device(const Window& window, PhysicalDeviceType type = INTEL);
        Device(const Device &) = delete;
        Device &operator=(const Device &) = delete;
        Device(Device &&) = delete;
        Device &operator=(Device &&) = delete;

        ~Device();

        [[nodiscard]] VkInstance instance() const { return m_instance; }
        [[nodiscard]] VkDevice device() const { return m_device; }
        [[nodiscard]] VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
        [[nodiscard]] VkSurfaceKHR surface() const { return m_surface; }
        [[nodiscard]] VkCommandPool commandPool() const { return m_commandPool; }
        [[nodiscard]] VkQueue graphicsQueue() const { return m_graphicsQueue; }
        [[nodiscard]] VkQueue presentQueue() const { return m_presentQueue; }
        [[nodiscard]] VkSampleCountFlagBits msaaSamples() const { return m_msaaSamples; }

        // helper functions
        [[nodiscard]] QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) const;
        [[nodiscard]] uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
        void executeSingleCommand(const std::function<void(VkCommandBuffer&)>& function) const;
        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const;
        void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const;
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
        [[nodiscard]] SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice) const;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        // helper functions
        bool checkValidationLayerSupport();
        [[nodiscard]] std::vector<const char*> getRequiredExtension() const;
        static void checkNecessaryExtensions(const char** glfwExtensions, uint32_t count);
        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        bool isDeviceSuitable(VkPhysicalDevice pDevice);
        bool checkDeviceExtensionSupport(VkPhysicalDevice pDevice);
        VkSampleCountFlagBits getMaxUsableSampleCount();

        const Window& m_windowRef;

        PhysicalDeviceType m_deviceType;
        VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        VkInstance m_instance{};
        VkDebugUtilsMessengerEXT m_debugMessenger{};
        VkSurfaceKHR m_surface{};
        VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
        VkDevice m_device{};
        VkQueue m_graphicsQueue{};
        VkQueue m_presentQueue{};
        VkCommandPool m_commandPool{};

        const std::vector<const char*> validationLayers = {
                "VK_LAYER_KHRONOS_validation"
        };

        const std::vector<const char*> deviceExtensions = {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };
    };
}


#endif //VULKANTUTORIAL_DEVICE_H
