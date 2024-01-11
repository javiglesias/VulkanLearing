run: VulkanLearning
	./VulkanLearning.out
VulkanLearning: shaders
	g++ pruebaVulkan/main.cpp -o VulkanLearning.out -I"dependencies/Vulkan_1.3.268.0/x86_64/include" -I"dependencies/glfw/include" -L"dependencies/Vulkan_1.3.268.0/x86_64/lib" -std=c++17 -lglfw -lvulkan -ldl -lpthread -lX11 > compilation.log
	
shaders:
	./dependencies/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.vert -o resources/Shaders/vert.spv
	./dependencies/Vulkan_1.3.268.0/x86_64/bin/glslc resources/Shaders/Standard.frag -o resources/Shaders/frag.spv
		
clean:
	rm -f VulkanLearning.out
