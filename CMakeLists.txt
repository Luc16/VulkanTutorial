cmake_minimum_required(VERSION 3.22)
project(VulkanTutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED )

add_executable(VulkanTutorial main.cpp)
target_link_libraries(VulkanTutorial PRIVATE GL glfw Vulkan::Vulkan)
