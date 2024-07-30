SRCS := $(shell find ./engine -name '*.cpp')
SPV := $(shell find ./dependencies/glslang/SPIRV -name '*.cpp')
CFLAGS = -g -v -fstack-usage -std=gnu++17 -Wc++17-extensions
LIBS = -lglfw  -lvulkan -ldl -lpthread -lX11 -lassimp dependencies/glslang/glslang/libglslang.a dependencies/glslang/SPIRV/libSPIRV.a dependencies/glslang/SPIRV/libSPVRemapper.a
VulkanLearning: shaders
	clang++  $(CFLAGS)  $(SRCS) dependencies/imgui/imgui.cpp dependencies/imgui/imgui_draw.cpp dependencies/imgui/imgui_widgets.cpp dependencies/imgui/imgui_tables.cpp dependencies/imgui/backends/imgui_impl_glfw.cpp dependencies/imgui/backends/imgui_impl_vulkan.cpp -o VulkanLearning.out -I"dependencies/glfw/include" -I"dependencies/assimp/include" -I"dependencies/glslang/glslang/Public" -I"dependencies/glslang/SPIRV"  -L"dependencies/assimp/bin" -L"dependencies/glfw/src" $(LIBS)
shaders:
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=450 engine/shaders/Standard.vert -o engine/shaders/vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc --target-env=vulkan1.3 -std=450 engine/shaders/Standard.frag -o engine/shaders/frag.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.vert -o engine/shaders/dbgVert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc engine/shaders/Debug.frag -o engine/shaders/dbgFrag.spv
clean:
	rm -f VulkanLearning.out
