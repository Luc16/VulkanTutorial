cmake_minimum_required(VERSION 3.22)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 20)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED )

file(GLOB imgui CONFIGURE_DEPENDS "external/imgui/*.h" "external/imgui/*.cpp")
#add_executable(VulkanTutorial main.cpp external/imgui/imgui.h external/imgui/imgui.cpp external/imgui/imgui_demo.cpp external/imgui/imgui_impl_glfw.h external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_vulkan.h external/imgui/imgui_impl_vulkan.cpp )
add_executable(VulkanTutorial main.cpp external/stbimage/stb_image.h ${imgui} external/objloader/tiny_obj_loader.h src/lib/SwapChain.cpp src/lib/SwapChain.h src/lib/utils.cpp src/lib/utils.h src/lib/Buffer.cpp src/lib/Buffer.h src/lib/Device.cpp src/lib/Device.h src/lib/Window.cpp src/lib/Window.h src/lib/Image.cpp src/lib/Image.h src/lib/Model.cpp src/lib/Model.h src/lib/Texture.cpp src/lib/Texture.h src/lib/Pipeline.cpp src/lib/Pipeline.h src/lib/Renderer.cpp src/lib/Renderer.h src/lib/descriptors/DescriptorSetLayout.cpp src/lib/descriptors/DescriptorSetLayout.h src/lib/descriptors/DescriptorPool.cpp src/lib/descriptors/DescriptorPool.h src/lib/descriptors/DescriptorWriter.cpp src/lib/descriptors/DescriptorWriter.h src/lib/CameraMovementController.cpp src/lib/CameraMovementController.h src/lib/Camera.cpp src/lib/Camera.h src/lib/DrawableObject.cpp src/lib/DrawableObject.h src/lib/RenderSystem.cpp src/lib/RenderSystem.h src/lib/VulkanApp.cpp src/lib/VulkanApp.h src/lib/InstancedObjects.cpp src/lib/InstancedObjects.h DefaultApp.cpp DefaultApp.h InstancingApp.cpp InstancingApp.h)
target_link_libraries(VulkanTutorial PRIVATE GL glfw Vulkan::Vulkan)
