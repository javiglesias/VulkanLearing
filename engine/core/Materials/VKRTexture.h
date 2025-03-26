#ifndef _C_TEXTURE
#define _C_TEXTURE
#include <string>
#include <vulkan/vulkan_core.h>

typedef struct VmaAllocation_T* VmaAllocation;

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		struct vk_Allocated_Image
		{
			VkImage image;
			VkImageView view;
			VmaAllocation allocation;
			VkDeviceMemory memory;
			VkExtent3D extent;
			VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
		};
		struct Texture
		{
		private: // Variables
			VkBuffer m_StagingBuffer;
			VkDeviceMemory m_StaggingBufferMemory;
		public:
			uint8_t m_Mipmaps = 1;
			char m_Path[256];
			vk_Allocated_Image vk_image;
			VkSampler m_Sampler = nullptr;
		public:
			Texture(std::string _path);
			void LoadTexture();
			void LoadCubemapTexture();

			void CreateImage(VkExtent3D _extent, VkFormat _format, VkImageTiling _tiling
				, VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties
				, uint32_t _arrayLayers, VkImageCreateFlags _flags, uint8_t _mipmapLvls=1);
			void CreateAndTransitionImage(VkCommandPool _CommandPool
				, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB 
				, VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
				, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
				, uint32_t _arrayLayers=1, VkImageCreateFlags _flags=0);
			void CreateAndTransitionImageNoMipMaps(VkCommandPool _CommandPool
				, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB
				, VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
				, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
				, uint32_t _arrayLayers=1, VkImageCreateFlags _flags=0);
			void CreateAndTransitionImageCubemap(VkCommandPool _CommandPool
				, VkFormat _format = VK_FORMAT_R8G8B8A8_SRGB
				, VkImageAspectFlags _aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
				, VkImageViewType _viewType = VK_IMAGE_VIEW_TYPE_2D
				, uint32_t _arrayLayers = 1, VkImageCreateFlags _flags = 0);

			void GenerateMipmap(VkCommandPool _CommandPool, uint8_t _mipLevels);
			void TransitionImageLayout(VkImageLayout _old, VkImageLayout _new,
			                           VkCommandPool _CommandPool, uint32_t _layerCount, VkQueue* _queue,
			                           uint8_t _levelCount = 1);
			void CopyBufferToImage(VkBuffer _buffer,VkExtent3D _extent, VkDeviceSize _bufferOffset
					, VkCommandPool _CommandPool, VkQueue* _queue, uint32_t _layer = 0);
			void CreateImageView(VkImageAspectFlags _aspectMask,VkImageViewType _viewType
						, uint32_t _arrayLayers = 1, uint32_t _levelCount = 1);
			void CreateTextureImageView();
			void BindTextureMemory();
			VkSampler CreateTextureSampler(float _Mipmaps,
				VkSamplerAddressMode _u = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				VkSamplerAddressMode _v = VK_SAMPLER_ADDRESS_MODE_REPEAT,
				VkSamplerAddressMode _w = VK_SAMPLER_ADDRESS_MODE_REPEAT);
			void CreateShadowTextureSampler();
			void CleanTextureData(VkDevice _LogicDevice);
		};
	}
}
#endif