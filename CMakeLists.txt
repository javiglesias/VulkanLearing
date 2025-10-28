cmake_minimum_required(VERSION 3.3...3.5)
set(CMAKE_CXX_COMPILER g++)

project(VKRenderer VERSION 0.1 DESCRIPTION "A Vulkan Renderer" LANGUAGES CXX)
if(MSVC)
set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS")
add_compile_definitions(_MSVC)
endif (MSVC)
if (WIN32)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/windows/Vulkan_1.3.268.0/Include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/cpptrace/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glm-0.9.9.0/glm)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/assimp/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/vma/include)
endif (WIN32)

if (UNIX)
add_compile_definitions(_UNIX)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/linux/vulkan_1.3.296.0/x86_64/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glslang)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glm-0.9.9.0/glm)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/assimp/include)
endif (UNIX)

file(GLOB_RECURSE SOURCES engine/*.cpp)
	
add_executable(VKRenderer ${SOURCES}
        dependencies/imgui/imgui.cpp
        dependencies/imgui/imgui_draw.cpp
        dependencies/imgui/imgui_tables.cpp
        dependencies/imgui/imgui_widgets.cpp
		dependencies/imgui/backends/imgui_impl_win32.cpp
        dependencies/imgui/backends/imgui_impl_glfw.cpp
        dependencies/imgui/backends/imgui_impl_vulkan.cpp
)
target_compile_features(VKRenderer PRIVATE cxx_std_20)
if (WIN32)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/assimp/lib/Debug/assimp-vc143-mtd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/windows/Vulkan_1.3.268.0/Lib/vulkan-1.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glfw/src/Debug/glfw3.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/glslangd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/glslang-default-resource-limitsd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/MachineIndependentd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/GenericCodeGend.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/Debug/SPIRVd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/Debug/SPVRemapperd.lib)
endif (WIN32)

if (UNIX)
add_compile_definitions(_UNIX)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/assimp/bin/libassimp.so)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/linux/vulkan_1.3.296.0/x86_64/lib/libvulkan.so)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glfw/src/libglfw3.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/libglslang.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/libglslang-default-resource-limits.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/libMachineIndependent.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/libGenericCodeGen.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/libSPIRV.a)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/libSPVRemapper.a)
endif (UNIX)
set(CMAKE_BUILD_TYPE Debug)
