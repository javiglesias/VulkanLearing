#pragma once
#include <string>
#include <vulkan/vulkan_core.h>

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		extern const int FRAMES_IN_FLIGHT;

		struct Texture
		{
		private: // Variables
			int m_DefualtWidth, m_DefualtHeight, m_DefualtChannels;
			VkBuffer m_StagingBuffer;
			VkDeviceMemory m_StaggingBufferMemory;
		public:
			std::string sPath;
			VkImage tImage = nullptr;
			VkDeviceMemory tImageMem = nullptr;
			VkImageView tImageView = nullptr;
			VkSampler m_Sampler = nullptr;
		public:
			Texture() {}
			Texture(std::string _path)
			{
				sPath = _path;
			}
			void CreateAndTransitionImage(VkCommandPool _CommandPool);
			void CleanTextureData(VkDevice _LogicDevice);
		};
	}
}