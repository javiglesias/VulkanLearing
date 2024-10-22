SRCS := $(shell find /engine -name '*.cpp')
OBJS := $(SRCS:%=./engine/%.o)
CXX = clang++
SPV := $(shell find /dependencies/glslang -name '*.cpp')

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

objects:
	$(CXX) $(CFLAGS) -c $(SRCS) $(INC) $(DLIBS) $(LIBS)

shaders:
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=460 engine/shaders/Standard.vert -o engine/shaders/Standard.vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=460 engine/shaders/Standard.frag -o engine/shaders/Standard.frag.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.vert -o engine/shaders/Debug.vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.frag -o engine/shaders/Debug.frag.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Shadow.vert -o engine/shaders/Shadow.vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Cubemap.vert -o engine/shaders/Cubemap.vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Cubemap.frag -o engine/shaders/Cubemap.frag.spv
.PHONY: clean
clean:
	rm -f VulkanLearning.out
	rm -f *.su
