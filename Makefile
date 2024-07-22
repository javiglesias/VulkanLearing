VulkanLearning:
	g++ -g -fstack-usage engine/main.cpp dependencies/imgui/imgui.cpp dependencies/imgui/imgui_draw.cpp dependencies/imgui/imgui_widgets.cpp dependencies/imgui/imgui_tables.cpp dependencies/imgui/backends/imgui_impl_glfw.cpp dependencies/imgui/backends/imgui_impl_vulkan.cpp -o VulkanLearning.out -I"dependencies/glfw/include" -I"dependencies/assimp/include" -L"dependencies/assimp/bin" -lassimp -std=c++17 -lglfw  -lvulkan -ldl -lpthread -lX11
run:  VulkanLearning shaders
	./VulkanLearning.out Sponza/glTF Sponza.gltf
clean:
	rm -f VulkanLearning.out
