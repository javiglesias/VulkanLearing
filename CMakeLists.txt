cmake_minimum_required(VERSION 3.3...3.5)
set(CMAKE_CXX_COMPILER g++)

project(VKRenderer VERSION 0.1 DESCRIPTION "A Vulkan Renderer" LANGUAGES CXX)
set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS")
if (WIN32)
add_compile_definitions(_WIN32)
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
file(GLOB SOURCES
	engine/*.cpp
	engine/*.h)
	
add_executable(VKRenderer engine/main.cpp
        engine/core/VKRenderers.cpp
        engine/core/VKRenderers.h
        engine/core/VKRRenderPass.cpp
        engine/core/VKRRenderPass.h
        engine/core/VKRScene.cpp
        engine/core/VKRScene.h
        engine/core/CVarSystem
        engine/core/Materials/VKRDebugMaterial.cpp
        engine/core/Materials/VKRDebugMaterial.h
        engine/core/Materials/VKRCubemapMaterial.cpp
        engine/core/Materials/VKRCubemapMaterial.h
        engine/core/Materials/VKRMaterial.cpp
        engine/core/Materials/VKRMaterial.h
        engine/core/Materials/VKRShadowMaterial.h
        engine/core/Materials/VKRShader.cpp
        engine/core/Materials/VKRShader.h
        engine/core/Materials/VKRTexture.cpp
        engine/core/Materials/VKRTexture.h
        engine/core/Objects/VKRCubemap.cpp
        engine/core/Objects/VKRCubemap.h
        engine/core/Objects/VKRCamera.h
        engine/core/Objects/VKRLight.cpp
        engine/core/Objects/VKRLight.h
        engine/core/Objects/VKRModel.cpp
        engine/core/Objects/VKRModel.h
        engine/editor/Editor.cpp
        engine/editor/Editor.h
        engine/perfmon/Custom.h
        engine/filesystem/shader_compiler.cpp
        engine/filesystem/gltfReader.cpp
        engine/filesystem/gltfReader.h
        engine/filesystem/ResourceManager.cpp
        engine/filesystem/ResourceManager.h
        engine/video/VKBackend.cpp
        engine/video/VKBackend.h
        engine/video/VKRenderable.cpp
        engine/video/VKRenderable.h
        engine/video/VKBufferObjects.h
        engine/video/VKRUtils.h
        engine/video/VKRUtils.cpp
        engine/video/Types.h
        engine/video/GPUParticle.h
        engine/video/VKRenderCommon.h
        engine/memory/mem_alloc.h
        dependencies/imgui/imgui.cpp
        dependencies/imgui/imgui_draw.cpp
        dependencies/imgui/imgui_tables.cpp
        dependencies/imgui/imgui_widgets.cpp
        dependencies/imgui/backends/imgui_impl_glfw.cpp
        dependencies/imgui/backends/imgui_impl_vulkan.cpp
)
target_compile_features(VKRenderer PRIVATE cxx_std_20)
if (WIN32)
add_compile_definitions(_WIN32)
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