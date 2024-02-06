VulkanLearning:
	g++ -g pruebaVulkan/main.cpp dependencies/imgui/imgui.cpp dependencies/imgui/imgui_draw.cpp dependencies/imgui/imgui_widgets.cpp dependencies/imgui/imgui_tables.cpp dependencies/imgui/backends/imgui_impl_glfw.cpp dependencies/imgui/backends/imgui_impl_vulkan.cpp -o VulkanLearning.out -I"dependencies/glfw/include" -I"dependencies/assimp/include" -L"dependencies/assimp/bin" -lassimp -std=c++17 -lglfw  -lvulkan -ldl -lpthread -lX11
run:  shaders
	./VulkanLearning.out scene/glTF scene.gltf
shaders:
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.vert -o resources/Shaders/vert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.frag -o resources/Shaders/frag.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Debug.vert -o resources/Shaders/dbgVert.spv
	./build/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Debug.frag -o resources/Shaders/dbgFrag.spv

clean:
	rm -f VulkanLearning.out
