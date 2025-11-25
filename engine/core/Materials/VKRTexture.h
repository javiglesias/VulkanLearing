#ifndef _C_TEXTURE
#define _C_TEXTURE
#include <string>
#include <vulkan/vulkan.h>

typedef struct VmaAllocation_T* VmaAllocation;

namespace VKR
{
	namespace render
	{
		struct vk_Allocated_Image
		{
			VkImage image;
			VkImageView view;
			VmaAllocation allocation;
#ifndef USE_VMA
			VkDeviceMemory memory;
#else
			VmaAllocation memory;
#endif
			VkExtent3D extent;
			VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
		};
		struct Texture
		{

			VkBuffer m_StagingBuffer;
			VkDeviceMemory m_StaggingBufferMemory;

			uint8_t m_Mipmaps = 1;
			char m_Path[256];
			vk_Allocated_Image vk_image;
			VkSampler m_Sampler = nullptr;
			Texture() {}
			Texture(vk_Allocated_Image _image, VkSampler _sampler);
			void init(std::string);
		};
	}
}
#endif