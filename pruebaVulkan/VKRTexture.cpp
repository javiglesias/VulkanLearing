#include "VKRTexture.h"
#include "VKRUtils.h"
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "../dependencies/stb/stb_image.h"

namespace VKR
{
	namespace render
	{
		Texture::Texture(std::string _path)
		{
			if (_path.empty())
				m_Path = std::string("resources/Textures/checkerW.png");
			else
				m_Path = _path;
		}
		void Texture::LoadTexture(bool isCubemap)
		{
			stbi_uc* pixels;

			pixels = stbi_load(m_Path.c_str(), &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			if (!pixels)
			{
				printf("\rMissing Texture %s\n", m_Path.c_str());
				stbi_uc* m_DefaultTexture;
				m_DefaultTexture = stbi_load("resources/Textures/checkerW.png", &m_DefualtWidth, &m_DefualtHeight, &m_DefualtChannels, STBI_rgb_alpha);
				pixels = m_DefaultTexture;
				tWidth = m_DefualtWidth;
				tHeight = m_DefualtHeight;
			}
			m_Size = tWidth * tHeight * 4;
			if (isCubemap)
				m_Size *= 6;
			// Buffer transfer of pixels
			CreateBuffer(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_StagingBuffer, m_StaggingBufferMemory);
			void* data;
			vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, m_Size, 0, &data);
			if (isCubemap)
				m_Size /= 6;
			memcpy(data, pixels, (size_t)m_Size);
			vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
			stbi_image_free(pixels);

		}

		void Texture::CreateAndTransitionImage(VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask, VkImageViewType _viewType,
			uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			// Como utilizamos lightview, las texturas solo se crean una vez.
			if (tImageView == nullptr && m_Sampler == nullptr
				&& tImage == nullptr && tImageMem == nullptr)
			{
				auto texSize = tWidth * tHeight * 4;
				CreateImage(tWidth, tHeight, _format,
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tImage, &tImageMem, _arrayLayers, _flags);
				vkBindImageMemory(g_context.m_LogicDevice, tImage, tImageMem, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_CommandPool, _arrayLayers);
				CopyBufferToImage(m_StagingBuffer, tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0, _CommandPool, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					_CommandPool, _arrayLayers);
				tImageView = CreateImageView(tImage, _format, _aspectMask, _viewType, _arrayLayers);
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
