cmake_minimum_required(VERSION 3.22)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED )

file(GLOB imgui CONFIGURE_DEPENDS "external/imgui/*.h" "external/imgui/*.cpp")
#add_executable(VulkanTutorial main.cpp external/imgui/imgui.h external/imgui/imgui.cpp external/imgui/imgui_demo.cpp external/imgui/imgui_impl_glfw.h external/imgui/imgui_impl_glfw.cpp external/imgui/imgui_impl_vulkan.h external/imgui/imgui_impl_vulkan.cpp )
add_executable(VulkanTutorial main.cpp ${imgui})
target_link_libraries(VulkanTutorial PRIVATE GL glfw Vulkan::Vulkan)
