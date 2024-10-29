cmake_minimum_required(VERSION 3.5)
project(VKRenderer
        VERSION 0.1
        DESCRIPTION "A Vulkan Renderer"
        LANGUAGES CXX)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/Vulkan_1.3.268.0/Include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/cpptrace/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glfw/include)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/glm-0.9.9.0/glm)
include_directories(${PROJECT_SOURCE_DIR}/dependencies/assimp/include)
add_executable(VKRenderer engine/main.cpp
        engine/core/VKRenderers.cpp
        engine/core/VKRRenderPass.cpp
        engine/core/VKRScene.cpp
        engine/core/Materials/VKRCubemapMaterial.cpp
        engine/core/Materials/VKRMaterial.cpp
        engine/core/Materials/VKRShader.cpp
        engine/core/Materials/VKRTexture.cpp
        engine/core/Objects/VKRCubemap.cpp
        engine/core/Objects/VKRLight.cpp
        engine/core/Objects/VKRModel.cpp
        engine/editor/Editor.cpp
        engine/perfmon/Custom.h
        engine/filesystem/gltfReader.cpp
        engine/filesystem/ResourceManager.cpp
        engine/video/VKBackend.cpp
        engine/video/VKRenderable.cpp
        dependencies/imgui/imgui.cpp
        dependencies/imgui/imgui_draw.cpp
        dependencies/imgui/imgui_tables.cpp
        dependencies/imgui/imgui_widgets.cpp
        dependencies/imgui/backends/imgui_impl_glfw.cpp
        dependencies/imgui/backends/imgui_impl_vulkan.cpp
        dependencies/cgltf/cgltf.h
)
target_compile_features(VKRenderer PRIVATE cxx_std_20)

target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/assimp/lib/Debug/assimp-vc143-mtd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/Vulkan_1.3.268.0/Lib/vulkan-1.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glfw/src/Debug/glfw3.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/glslangd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/glslang-default-resource-limitsd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/MachineIndependentd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/glslang/Debug/GenericCodeGend.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/Debug/SPIRVd.lib)
target_link_libraries(VKRenderer PRIVATE ${PROJECT_SOURCE_DIR}/dependencies/glslang/SPIRV/Debug/SPVRemapperd.lib)
