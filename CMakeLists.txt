cmake_minimum_required(VERSION 3.22)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED )

file(GLOB imgui CONFIGURE_DEPENDS "external/imgui/*.h" "external/imgui/*.cpp")
#add_executable(VulkanTutorial main.cpp external/imgui/imgui.h external/imgui/imgui.cpp external/imgui/imgui_demo.cpp external/imgui/imgui_impl_glfw.h external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_vulkan.h external/imgui/imgui_impl_vulkan.cpp )
add_executable(VulkanTutorial main.cpp external/stbimage/stb_image.h ${imgui} external/objloader/tiny_obj_loader.h SwapChain.cpp SwapChain.h utils.cpp utils.h Buffer.cpp Buffer.h Device.cpp Device.h Window.cpp Window.h Image.cpp Image.h Model.cpp Model.h Texture.cpp Texture.h Pipeline.cpp Pipeline.h Renderer.cpp Renderer.h descriptors/DescriptorSetLayout.cpp descriptors/DescriptorSetLayout.h descriptors/DescriptorPool.cpp descriptors/DescriptorPool.h descriptors/DescriptorWriter.cpp descriptors/DescriptorWriter.h CameraMovementController.cpp CameraMovementController.h Camera.cpp Camera.h DrawableObject.cpp DrawableObject.h RenderSystem.cpp RenderSystem.h VulkanApp.cpp VulkanApp.h)
target_link_libraries(VulkanTutorial PRIVATE GL glfw Vulkan::Vulkan)
