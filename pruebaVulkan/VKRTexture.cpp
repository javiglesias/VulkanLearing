#include "VKRTexture.h"
#include "VKRUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb/stb_image.h"

namespace VKR
{
	namespace render
	{
		void Texture::CreateAndTransitionImage(VkCommandPool _CommandPool)
		{
			// Como utilizamos lightview, las texturas solo se crean una vez.
			if (tImageView == nullptr && m_Sampler == nullptr
				&& tImage == nullptr && tImageMem == nullptr)
			{
				int tWidth, tHeight, tChannels;
				stbi_uc* pixels;
				VkDeviceSize tSize;
				pixels = stbi_load(sPath.c_str(), &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
				if (!pixels)
				{
					printf("\rMissing Texture %s\n", sPath.c_str());
					stbi_uc* m_DefaultTexture;
					m_DefaultTexture = stbi_load("resources/Textures/checkerW.png", &m_DefualtWidth, &m_DefualtHeight, &m_DefualtChannels, STBI_rgb_alpha);
					pixels = m_DefaultTexture;
					tWidth = m_DefualtWidth;
					tHeight = m_DefualtHeight;
				}
				tSize = tWidth * tHeight * 4;
				// Buffer transfer of pixels
				CreateBuffer(tSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_StagingBuffer, m_StaggingBufferMemory);
				void* data;
				vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, tSize, 0, &data);
				memcpy(data, pixels, (size_t)tSize);
				vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
				stbi_image_free(pixels);

				auto texSize = tWidth * tHeight * 4;
				CreateImage(tWidth, tHeight, VK_FORMAT_R8G8B8A8_SRGB,
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tImage, &tImageMem);
				vkBindImageMemory(g_context.m_LogicDevice, tImage, tImageMem, 0);
				TransitionImageLayout(tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_CommandPool);
				CopyBufferToImage(m_StagingBuffer, tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0, _CommandPool);
				TransitionImageLayout(tImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					_CommandPool);
				tImageView = CreateTextureImageView(tImage);
				m_Sampler = CreateTextureSampler();
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
			}
		}

		void Texture::CleanTextureData(VkDevice _LogicDevice)
		{
			vkDestroySampler(_LogicDevice, m_Sampler, nullptr);
			m_Sampler = nullptr;
			vkDestroyImageView(_LogicDevice, tImageView, nullptr);
			tImageView = nullptr;
			vkDestroyImage(_LogicDevice, tImage, nullptr);
			tImage = nullptr;
			vkFreeMemory(_LogicDevice, tImageMem, nullptr);
			tImageMem = nullptr;
		}
	}
}
