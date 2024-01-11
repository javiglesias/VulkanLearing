run: VulkanLearning
	./VulkanLearning.out
VulkanLearning: shaders
	g++ pruebaVulkan/main.cpp -o VulkanLearning.out -I"dependencies/glfw/include" -std=c++17 -lglfw -lvulkan -ldl -lpthread -lX11
	
shaders:
	./dependencies/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.vert -o resources/Shaders/vert.spv
	./dependencies/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.frag -o resources/Shaders/frag.spv
		
clean:
	rm -f VulkanLearning.out
