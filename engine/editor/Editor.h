#ifndef _C_EDITOR
#define _C_EDITOR

#include <vector>
#include <vulkan/vulkan.h>
#include <sys/types.h>

struct GLFWwindow;

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
			std::vector<char*> m_Models;
		public:
			Editor(GLFWwindow* _Window, VkInstance _Instance, uint32_t _MinImageCount, uint32_t _ImageCount);
			void Cleanup();
			void Shutdown();
			~Editor();
			void Loop(Scene* _mainScene, VKBackend* _backend);
			void Draw(VkCommandBuffer _commandBuffer);
		};
		//static Editor* g_editor;
	}
}
#endif