#pragma once

#include "../../dependencies/imgui/misc/single_file/imgui_single_file.h"
#include "../../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../../dependencies/imgui/backends/imgui_impl_vulkan.h"

namespace VKR
{
	namespace render
	{
		class VKBackend;
		class Scene;
		class Editor
		{
		private:
			VkDescriptorPool m_UIDescriptorPool;
		public:
			Editor(GLFWwindow* _Window, VkInstance _Instance, uint32_t _MinImageCount, uint32_t _ImageCount);
			void Cleanup();
			void Shutdown();
			~Editor();
			void Loop(Scene* _mainScene, VKBackend* _backend);
			void Draw(VkCommandBuffer _commandBuffer);
		};	
	}
}