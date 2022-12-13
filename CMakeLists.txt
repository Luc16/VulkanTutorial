cmake_minimum_required(VERSION 3.22)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED )

file(GLOB imgui CONFIGURE_DEPENDS "external/imgui/*.h" "external/imgui/*.cpp")
#add_executable(VulkanTutorial main.cpp external/imgui/imgui.h external/imgui/imgui.cpp external/imgui/imgui_demo.cpp external/imgui/imgui_impl_glfw.h external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_vulkan.h external/imgui/imgui_impl_vulkan.cpp )
add_executable(VulkanTutorial main.cpp external/stbimage/stb_image.h ${imgui} external/objloader/tiny_obj_loader.h src/SwapChain.cpp src/SwapChain.h src/utils.cpp src/utils.h src/Buffer.cpp src/Buffer.h src/Device.cpp src/Device.h src/Window.cpp src/Window.h src/Image.cpp src/Image.h src/Model.cpp src/Model.h src/Texture.cpp src/Texture.h src/Pipeline.cpp src/Pipeline.h src/Renderer.cpp src/Renderer.h src/descriptors/DescriptorSetLayout.cpp src/descriptors/DescriptorSetLayout.h src/descriptors/DescriptorPool.cpp src/descriptors/DescriptorPool.h src/descriptors/DescriptorWriter.cpp src/descriptors/DescriptorWriter.h src/CameraMovementController.cpp src/CameraMovementController.h src/Camera.cpp src/Camera.h src/DrawableObject.cpp src/DrawableObject.h src/RenderSystem.cpp src/RenderSystem.h src/VulkanApp.cpp src/VulkanApp.h src/InstancedObjects.cpp src/InstancedObjects.h DefaultApp.cpp DefaultApp.h InstancingApp.cpp InstancingApp.h)
target_link_libraries(VulkanTutorial PRIVATE GL glfw Vulkan::Vulkan)
