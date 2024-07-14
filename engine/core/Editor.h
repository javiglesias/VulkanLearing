#pragma once

#include "../../dependencies/imgui/misc/single_file/imgui_single_file.h"
#include "../../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../../dependencies/imgui/backends/imgui_impl_vulkan.h"
#include "Resource.h"

namespace VKR
{
	struct sResource;
	void LoadAsyncResource(const char* _path);
	void AddAsyncRequest(sResource* _request);
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
