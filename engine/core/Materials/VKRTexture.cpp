#include "VKRTexture.h"
#include "../../video/VKRUtils.h"
#include "../../perfmon/Custom.h"

#include <string>
#include <cmath>

#define STB_IMAGE_IMPLEMENTATION
#include "../../../dependencies/stb/stb_image.h"

namespace VKR
{
	namespace render
	{
		Texture::Texture(std::string _path)
		{
			if (_path.empty())
				sprintf(m_Path, "resources/Textures/checkerW.png");
			else
				sprintf(m_Path, "%s", _path.c_str());
			PERF_INIT("LOAD_TEXTURE")
			stbi_uc* pixels = nullptr;
			stbi_set_flip_vertically_on_load(true);
			if (!m_Path[0] == '\0')
				pixels = stbi_load(m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			if (!pixels)
			{
				//printf("\rMissing Texture %s\n", m_Path);
				stbi_uc* m_DefaultTexture;
				m_DefaultTexture = stbi_load("resources/Textures/checkerW.png", &m_DefualtWidth, &m_DefualtHeight, &m_DefualtChannels, STBI_rgb_alpha);
				pixels = m_DefaultTexture;
				tWidth = m_DefualtWidth;
				tHeight = m_DefualtHeight;
			}
			PERF_END("LOAD_TEXTURE")
			m_Mipmaps = (uint8_t)std::log2(tWidth > tHeight ? tWidth : tHeight);
			m_Size = tWidth * tHeight * 4;
			m_Data = malloc((size_t)m_Size);
			memcpy(m_Data, pixels, (size_t)m_Size);
			stbi_image_free(pixels);
		}
		void Texture::LoadTexture(bool isCubemap)
		{
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
			memcpy(data, m_Data, (size_t)m_Size);
			vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
		}

		void Texture::LoadCubemapTexture()
		{
			stbi_uc* pixels;

			pixels = stbi_load(m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			if (!pixels)
			{
				printf("\rMissing Texture %s\n", m_Path);
				stbi_uc* m_DefaultTexture;
				m_DefaultTexture = stbi_load("resources/Textures/checkerW.png", &m_DefualtWidth, &m_DefualtHeight, &m_DefualtChannels, STBI_rgb_alpha);
				pixels = m_DefaultTexture;
				tWidth = m_DefualtWidth;
				tHeight = m_DefualtHeight;
			}
			tHeight=tWidth;
			m_Size = (tWidth * tHeight * 4);
			// Buffer transfer of pixels
			CreateBuffer(m_Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_StagingBuffer, m_StaggingBufferMemory);
			void* data;
			vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, m_Size, 0, &data);
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
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tImage, &tImageMem, _arrayLayers, _flags, m_Mipmaps);
				vkBindImageMemory(g_context.m_LogicDevice, tImage, tImageMem, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_CommandPool, _arrayLayers, 1);
				m_Mipmaps = 0;
				GenerateMipmap(tImage, _CommandPool, m_Mipmaps, tWidth, tHeight);
				/*for (size_t i = 0; i < _arrayLayers; i++)
				{
					auto offset = texSize * i;
				}*/
				auto levelCount = m_Mipmaps + 1;
				CopyBufferToImage(m_StagingBuffer, tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0, _CommandPool, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					_CommandPool, _arrayLayers, levelCount);
				tImageView = CreateImageView(tImage, _format, _aspectMask, _viewType, _arrayLayers, levelCount);
				m_Sampler = CreateTextureSampler(m_Mipmaps);
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
			}
		}

		void Texture::CreateAndTransitionImageNoMipMaps(VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask, VkImageViewType _viewType,
			uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			// Como utilizamos lightview, las texturas solo se crean una vez.
			if (tImageView == nullptr && m_Sampler == nullptr
				&& tImage == nullptr && tImageMem == nullptr)
			{
				auto texSize = tWidth * tHeight * 4;
				CreateImage(tWidth, tHeight, _format,
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tImage, &tImageMem, _arrayLayers, 0, 1);
				vkBindImageMemory(g_context.m_LogicDevice, tImage, tImageMem, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_CommandPool, _arrayLayers, 1);
				CopyBufferToImage(m_StagingBuffer, tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0, _CommandPool, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					_CommandPool, _arrayLayers, 1);
				tImageView = CreateImageView(tImage, _format, _aspectMask, _viewType, _arrayLayers, 1);
				m_Sampler = CreateTextureSampler(1);
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
			}
		}

		void Texture::CreateAndTransitionImageCubemap(VkCommandPool _CommandPool, VkFormat _format, 
													VkImageAspectFlags _aspectMask, VkImageViewType _viewType,
													uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			// Como utilizamos lightview, las texturas solo se crean una vez.
			if (tImageView == nullptr && m_Sampler == nullptr
				&& tImage == nullptr && tImageMem == nullptr)
			{
				auto texSize = tWidth * tHeight * 4;
				CreateImage(tWidth, tHeight, _format,
					VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &tImage, &tImageMem, _arrayLayers, 0, 1);
				// VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
				vkBindImageMemory(g_context.m_LogicDevice, tImage, tImageMem, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					_CommandPool, _arrayLayers, 1);
				CopyBufferToImage(m_StagingBuffer, tImage, static_cast<uint32_t>(tWidth), static_cast<uint32_t>(tHeight), 0, _CommandPool, 0);
				TransitionImageLayout(tImage, _format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
					_CommandPool, _arrayLayers, 1);
				tImageView = CreateImageView(tImage, _format, _aspectMask, _viewType, _arrayLayers, 1);
				m_Sampler = CreateTextureSampler(1, 
												VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
												VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
												VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
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
