SRCS := $(shell find ./engine -name '*.cpp')
SPV := $(shell find ./dependencies/glslang -name '*.cpp')
IMGUI = dependencies/imgui/imgui.cpp dependencies/imgui/imgui_draw.cpp dependencies/imgui/imgui_widgets.cpp dependencies/imgui/imgui_tables.cpp dependencies/imgui/backends/imgui_impl_glfw.cpp dependencies/imgui/backends/imgui_impl_vulkan.cpp
CFLAGS = -g -v -fstack-usage -std=gnu++17 -Wc++17-extensions
LIBS = -lglfw  -lvulkan -ldl -lpthread -lX11 -lassimp 
GLSLIBS = dependencies/glslang/glslang/libglslang.a dependencies/glslang/glslang/libMachineIndependent.a dependencies/glslang/glslang/libglslang-default-resource-limits.a dependencies/glslang/glslang/libGenericCodeGen.a
SPVLIBS = dependencies/glslang/External/spirv-tools/source/libSPIRV-Tools.a dependencies/glslang/SPIRV/libSPIRV.a dependencies/glslang/SPIRV/libSPVRemapper.a
INC = -I"dependencies/glfw/include" -I"dependencies/assimp/include" -I"dependencies/glslang/glslang/Public" -I"dependencies/glslang/SPIRV" -I"dependencies/glslang/include" -I"dependencies/glslang/External/spirv-tools/include"
DLIBS = -L"dependencies/assimp/bin" -L"dependencies/glfw/src" -L"dependencies/glslang/glslang" -L"dependencies/glslang/SPIRV" -L"dependencies/glslang/External/spirv-tools/source"
VulkanLearning: shaders
	clang++  $(CFLAGS)  $(SRCS) $(IMGUI) -o VulkanLearning.out $(INC) $(DLIBS) $(LIBS) $(GLSLIBS) $(SPVLIBS)
shaders:
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=450 engine/shaders/Standard.vert -o engine/shaders/vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=450 engine/shaders/Standard.frag -o engine/shaders/frag.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.vert -o engine/shaders/dbgVert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.frag -o engine/shaders/dbgFrag.spv
clean:
	rm -f VulkanLearning.out
	rm -f *.su
