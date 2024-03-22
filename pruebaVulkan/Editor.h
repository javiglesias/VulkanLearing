#pragma once

#include "../dependencies/imgui/misc/single_file/imgui_single_file.h"
#include "../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../dependencies/imgui/backends/imgui_impl_vulkan.h"

namespace VKR
{
	class Editor
	{
	public:
		Editor(GLFWwindow* _Window);
		~Editor();
		void Loop();
		void Draw(VkCommandBuffer _commandBuffer);
	};	
}