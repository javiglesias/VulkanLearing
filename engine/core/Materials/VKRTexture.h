#ifndef _C_TEXTURE
#define _C_TEXTURE
#include <string>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;

		struct Texture
		{
		private: // Variables
			VkBuffer m_StagingBuffer;
			VkDeviceMemory m_StaggingBufferMemory;
		public:
			uint8_t m_Mipmaps = 0;
			int tWidth, tHeight, tChannels;
			char m_Path[128] = "";
			VkImage tImage = nullptr;
			VkDeviceMemory tImageMem = nullptr;
			VkImageView tImageView = nullptr;
			VkSampler m_Sampler = nullptr;
		public:
			Texture(std::string _path);
			void LoadTexture();
			void LoadCubemapTexture();
			void CreateAndTransitionImage(VkCommandPool _CommandPool, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB,
				VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D,
				uint32_t _arrayLayers=1, VkImageCreateFlags _flags=0);
			void CreateAndTransitionImageNoMipMaps(VkCommandPool _CommandPool, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB,
				VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D,
				uint32_t _arrayLayers=1, VkImageCreateFlags _flags=0);
			void CreateAndTransitionImageCubemap(VkCommandPool _CommandPool, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB,
				VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D,
				uint32_t _arrayLayers=1, VkImageCreateFlags _flags=0);
			void CleanTextureData(VkDevice _LogicDevice);
		};
	}
}
#endif