OBJS := $(SRCS:%=./engine/%.o)
CXX = clang++

SRCS = engine/main.cpp engine/core/VKRenderers.cpp engine/core/VKRRenderPass.cpp engine/core/VKRScene.cpp engine/core/Materials/VKRCubemapMaterial.cpp engine/core/Materials/VKRMaterial.cpp engine/core/Materials/VKRShader.cpp engine/core/Materials/VKRTexture.cpp engine/core/Objects/VKRCubemap.cpp engine/core/Objects/VKRLight.cpp engine/core/Objects/VKRModel.cpp engine/editor/Editor.cpp engine/filesystem/gltfReader.cpp engine/filesystem/ResourceManager.cpp engine/video/VKBackend.cpp engine/video/VKRenderable.cpp
IMGUI = dependencies/imgui/imgui.cpp dependencies/imgui/imgui_draw.cpp dependencies/imgui/imgui_widgets.cpp dependencies/imgui/imgui_tables.cpp dependencies/imgui/backends/imgui_impl_glfw.cpp dependencies/imgui/backends/imgui_impl_vulkan.cpp
CGLTF = dependencies/cgltf/cgltf.h
CFLAGS = -g -std=gnu++17 -Wc++17-extensions
LIBS = -lglfw  -lvulkan -ldl -lpthread -lX11 -lassimp 

GLSLIBS = dependencies/glslang/glslang/libglslang.a dependencies/glslang/glslang/libMachineIndependent.a dependencies/glslang/glslang/libglslang-default-resource-limits.a dependencies/glslang/glslang/libGenericCodeGen.a
SPVLIBS = dependencies/glslang/External/spirv-tools/source/libSPIRV-Tools.a dependencies/glslang/SPIRV/libSPIRV.a dependencies/glslang/SPIRV/libSPVRemapper.a
INC = -I"dependencies/glfw/include" -I"dependencies/assimp/include" -I"dependencies/cgltf"
DLIBS = -L"dependencies/assimp/bin" -L"dependencies/glfw/src"
VulkanLearning:
	$(CXX) $(CFLAGS)  $(SRCS) $(IMGUI) -o VulkanLearning.out $(INC) $(DLIBS) $(LIBS)

.PHONY: clean
clean:
	rm -f VulkanLearning.out
	rm -f *.su
