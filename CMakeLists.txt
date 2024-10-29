cmake_minimum_required(VERSION 3.2)
project(VKRenderer)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/Vulkan_1.3.268.0/x86_64/include)
add_executable(VKRenderer engine/main.cpp)