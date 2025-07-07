#include "VKRTexture.h"
#include "../../video/VKRUtils.h"
#include "../../perfmon/Custom.h"

#ifndef WIN32
#include <signal.h>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "../../../dependencies/stb/stb_image.h"

#include <string>
#include <cmath>


namespace VKR
{
	namespace render
	{
		Texture::Texture(std::string _path)
		{
			memset(m_Path, 0, 256);
			if (_path.empty())
				sprintf(m_Path, "resources/Textures/defaultMissing.png");
			else
				sprintf(m_Path, "%s", _path.c_str());
		}

		Texture::Texture(vk_Allocated_Image _image, VkSampler _sampler)
		{
			vk_image = _image;
			m_Sampler = _sampler;
		}

		void Texture::LoadTexture()
		{
			PERF_INIT("LOAD_TEXTURE")
				stbi_uc* pixels = nullptr;
			stbi_set_flip_vertically_on_load(true);
			int tWidth, tHeight, tChannels;
			pixels = stbi_load(m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			vk_image.extent = VkExtent3D(tWidth, tHeight, 1);
			if (!pixels)
			{
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
				exit(-666);
			}
			PERF_END("LOAD_TEXTURE")
			m_Mipmaps = (uint8_t)std::log2(tWidth > tHeight ? tWidth : tHeight);
			size_t size = tWidth * tHeight * 4;
			fprintf(stderr, "\tLoading texture %s size: %llu\n", m_Path, size);
			// Buffer transfer of pixels
			utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				m_StagingBuffer, m_StaggingBufferMemory);
			void* data;
			vkMapMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory, 0, size, 0, &data);
			memcpy(data, pixels, size);
			vkUnmapMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory);
			stbi_image_free(pixels);
		}

		void Texture::LoadCubemapTexture()
		{
			LoadTexture();
			//stbi_uc* pixels;

			//pixels = stbi_load(m_Path, &tWidth, &tHeight, &tChannels, STBI_rgb_alpha);
			//if (!pixels)
			//{
			//	printf("\rMissing Texture %s\n", m_Path);
			//	#ifdef WIN32
			//		__debugbreak();
			//	#else
			//		raise(SIGTRAP);
			//	#endif
			//	exit(-666);
			//}
			//tHeight=tWidth;
			//auto size = (tWidth * tHeight * 4);
			//// Buffer transfer of pixels
			//utils::CreateBuffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
			//	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			//	m_StagingBuffer, m_StaggingBufferMemory);
			//void* data;
			//vkMapMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory, 0, size, 0, &data);
			//memcpy(data, pixels, static_cast<size_t>(size));
			//vkUnmapMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory);
			//stbi_image_free(pixels);

		}

		void Texture::CreateAndTransitionImage(VkCommandPool _CommandPool
			, VkFormat _format, VkImageAspectFlags _aspectMask, VkImageViewType _viewType
			, uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateImage(vk_image.extent, _format, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _arrayLayers, _flags, m_Mipmaps);
			BindTextureMemory();

			TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, _CommandPool, _arrayLayers, &utils::GetVKContext().m_GraphicsComputeQueue, m_Mipmaps);
			CopyBufferToImage(m_StagingBuffer, vk_image.extent, 0, _CommandPool
				, &utils::GetVKContext().m_GraphicsComputeQueue, 0);
			GenerateMipmap(_CommandPool, m_Mipmaps);
			TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				, _CommandPool, _arrayLayers, &utils::GetVKContext().m_GraphicsComputeQueue, m_Mipmaps);
			CreateImageView(_aspectMask, _viewType, _arrayLayers, m_Mipmaps);
			m_Sampler = CreateTextureSampler(m_Mipmaps);
			vkDestroyBuffer(utils::g_context.m_LogicDevice, m_StagingBuffer, nullptr);
			vkFreeMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
		}

		void Texture::CreateAndTransitionImageNoMipMaps(VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask
			, VkImageViewType _viewType , uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateImage(vk_image.extent, _format, VK_IMAGE_TILING_OPTIMAL
				, (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT)
				, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _arrayLayers, _flags, 1);
			BindTextureMemory();
			TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, _CommandPool, _arrayLayers, &utils::GetVKContext().m_GraphicsComputeQueue, 1);
			CopyBufferToImage(m_StagingBuffer, vk_image.extent, 0, _CommandPool
				, &utils::GetVKContext().m_GraphicsComputeQueue, 0);
			TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
				, _CommandPool, _arrayLayers, &utils::GetVKContext().m_GraphicsComputeQueue, 1);
			CreateImageView(_aspectMask, _viewType, _arrayLayers, 1);
			m_Sampler = CreateTextureSampler(1);
			vkDestroyBuffer(utils::g_context.m_LogicDevice, m_StagingBuffer, nullptr);
			vkFreeMemory(utils::g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
		}

		void Texture::CreateAndTransitionImageCubemap(VkCommandPool _CommandPool, VkFormat _format, VkImageAspectFlags _aspectMask
			, VkImageViewType _viewType, uint32_t _arrayLayers, VkImageCreateFlags _flags)
		{
			CreateAndTransitionImageNoMipMaps( _CommandPool, _format, _aspectMask
				, _viewType, _arrayLayers, _flags);
		}
		void Texture::CreateImage(VkExtent3D _extent, VkFormat _format, VkImageTiling _tiling
				, VkImageUsageFlagBits _usage, VkMemoryPropertyFlags _memProperties
				, uint32_t _arrayLayers, VkImageCreateFlags _flags, uint8_t _mipmapLvls)
		{
			auto renderContext = utils::GetVKContext();
			VkImageCreateInfo tImageInfo{};
			tImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			tImageInfo.imageType = VK_IMAGE_TYPE_2D;
			tImageInfo.extent = _extent;
			tImageInfo.extent.depth = 1;
			tImageInfo.mipLevels = _mipmapLvls;
			tImageInfo.arrayLayers = _arrayLayers;
			tImageInfo.format = _format;
			tImageInfo.tiling = _tiling;
			tImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			tImageInfo.usage = _usage;
			tImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			tImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			tImageInfo.flags = _flags;

			vk_image.extent = _extent;
			vk_image.format = _format;
#if not USE_VMA
			VK_ASSERT(vkCreateImage(renderContext.m_LogicDevice, &tImageInfo, nullptr, &vk_image.image));
			VkMemoryRequirements memRequ;
			vkGetImageMemoryRequirements(renderContext.m_LogicDevice, vk_image.image, &memRequ);
			VkMemoryAllocateInfo tAllocInfo{};
			tAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			tAllocInfo.allocationSize = memRequ.size;
			tAllocInfo.memoryTypeIndex = renderContext.m_GpuInfo.FindMemoryType(memRequ.memoryTypeBits, _memProperties);
			VK_ASSERT(vkAllocateMemory(renderContext.m_LogicDevice, &tAllocInfo, nullptr, &vk_image.memory));
#else
			utils::VMA_CreateImage(_memProperties, &tImageInfo, &vk_image.image, &vk_image.memory);
#endif
		}

		void Texture::GenerateMipmap(VkCommandPool _CommandPool, uint8_t _mipLevels)
		{
			auto renderContext = utils::GetVKContext();
			VkCommandBuffer commandBuffer = utils::BeginSingleTimeCommandBuffer(_CommandPool);
			VkImageMemoryBarrier iBarrier{};
			iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.image = vk_image.image;
			iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iBarrier.subresourceRange.layerCount = 1;
			iBarrier.subresourceRange.levelCount = 1;
			// copy/blit transactions
			for (uint8_t i = 1; i < _mipLevels; i++)
			{
				// pre-copy transition
				iBarrier.subresourceRange.baseMipLevel = i - 1;
				iBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				iBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				iBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
					1, &iBarrier);
				VkImageBlit blit{};
				blit.srcOffsets[0] = { 0, 0, 0 };
				blit.srcOffsets[1] = { (int32_t)vk_image.extent.width,  (int32_t)vk_image.extent.height, 1};
				blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.srcSubresource.mipLevel = i - 1;
				blit.srcSubresource.baseArrayLayer = 0;
				blit.srcSubresource.layerCount = 1;

				blit.dstOffsets[0] = { 0, 0, 0 };
				blit.dstOffsets[1] = { (int32_t)(vk_image.extent.width > 1 ? vk_image.extent.width/2 : 1)
					, (int32_t)(vk_image.extent.height > 1 ? vk_image.extent.height /2 : 1)
					, 1};
				blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				blit.dstSubresource.mipLevel = i;
				blit.dstSubresource.baseArrayLayer = 0;
				blit.dstSubresource.layerCount = 1;
				vkCmdBlitImage(commandBuffer, vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					VK_FILTER_LINEAR);
				vk_image.extent.width  = vk_image.extent.width > 1 ? vk_image.extent.width / 2 : 1;
				vk_image.extent.height = vk_image.extent.height > 1 ? vk_image.extent.height / 2 : 1;
				if (vk_image.extent.width == 1 || vk_image.extent.height == 1) 
					break;
			}
			// Post-copy transactions
			iBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			iBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			iBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
			vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,
				1, &iBarrier);
			utils::EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, renderContext.m_GraphicsComputeQueue);
		}

		void Texture::TransitionImageLayout(VkImageLayout _old, VkImageLayout _new, VkCommandPool _CommandPool
				, uint32_t _layerCount, VkQueue* _queue, uint8_t _levelCount)
		{
			auto renderContext = utils::GetVKContext();
			VkCommandBuffer commandBuffer = utils::BeginSingleTimeCommandBuffer(_CommandPool);
			VkImageMemoryBarrier iBarrier{};
			iBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			iBarrier.oldLayout = _old;
			iBarrier.newLayout = _new;
			iBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			iBarrier.image = vk_image.image;
			if (_new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (renderContext.HasStencilComponent(vk_image.format))
				{
					iBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				}
			}
			else
				iBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iBarrier.subresourceRange.baseMipLevel = 0;
			iBarrier.subresourceRange.levelCount = _levelCount;
			iBarrier.subresourceRange.baseArrayLayer = 0;
			iBarrier.subresourceRange.layerCount = _layerCount;
			VkPipelineStageFlags sourceStage;
			VkPipelineStageFlags destinationStage;

			if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
				iBarrier.srcAccessMask = 0;
				iBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (_old == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _new == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				iBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				iBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
				destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (_old == VK_IMAGE_LAYOUT_UNDEFINED && _new == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
			{
				iBarrier.srcAccessMask = 0;
				iBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

				sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
				destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			}
			else
			{
				printf("unsupported layout transition!");
				exit(-96);
			}
			vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &iBarrier);
			utils::EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, *_queue);
		}

		void Texture::CopyBufferToImage(VkBuffer _buffer, VkExtent3D _extent
				, VkDeviceSize _bufferOffset, VkCommandPool _CommandPool
				, VkQueue* _queue, uint32_t _layer)
		{
			VkCommandBuffer commandBuffer = utils::BeginSingleTimeCommandBuffer(_CommandPool);
			VkBufferImageCopy region{};
			region.bufferOffset = _bufferOffset;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = _layer;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = _extent;
			vkCmdCopyBufferToImage(commandBuffer, _buffer, vk_image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
			utils::EndSingleTimeCommandBuffer(commandBuffer, _CommandPool, *_queue);
		}

		void Texture::CreateImageView(VkImageAspectFlags _aspectMask, VkImageViewType _viewType
				, uint32_t _arrayLayers, uint32_t _levelCount)
		{
			auto renderContext = utils::GetVKContext();
			VkImageView tImageView;
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = vk_image.image;
			viewInfo.viewType = _viewType;
			viewInfo.format = vk_image.format;
			viewInfo.subresourceRange.aspectMask = _aspectMask;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = _levelCount;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = _arrayLayers;
			VK_ASSERT(vkCreateImageView(renderContext.m_LogicDevice, &viewInfo, nullptr, &vk_image.view));
		}

		void Texture::CreateTextureImageView()
		{
			// VK_FORMAT_R8G8B8A8_SRGB
			CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, 1);
		}

		void Texture::BindTextureMemory()
		{
#if not USE_VMA
			vkBindImageMemory(utils::g_context.m_LogicDevice, vk_image.image, vk_image.memory, 0);
#else
			utils::VMA_BindTextureMemory(vk_image.image, vk_image.memory);
#endif
		}

		VkSampler Texture::CreateTextureSampler(float _Mipmaps, VkSamplerAddressMode _u, VkSamplerAddressMode _v, VkSamplerAddressMode _w)
		{
			auto renderContext = utils::GetVKContext();
			VkPhysicalDeviceProperties deviceProp;
			VkSamplerCreateInfo samplerInfo{};
			VkSampler TextureSampler;
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = _u;
			samplerInfo.addressModeV = _v;
			samplerInfo.addressModeW = _w;
			samplerInfo.anisotropyEnable = VK_TRUE;
			vkGetPhysicalDeviceProperties(renderContext.m_GpuInfo.m_Device, &deviceProp);
			samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = _Mipmaps;
			VK_ASSERT(vkCreateSampler(renderContext.m_LogicDevice, &samplerInfo, nullptr, &TextureSampler));
			return TextureSampler;
		}

		void Texture::CreateShadowTextureSampler()
		{
			auto renderContext = utils::GetVKContext();
			VkPhysicalDeviceProperties deviceProp;
			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			samplerInfo.anisotropyEnable = VK_TRUE;

			vkGetPhysicalDeviceProperties(renderContext.m_GpuInfo.m_Device, &deviceProp);
			samplerInfo.maxAnisotropy = deviceProp.limits.maxSamplerAnisotropy;

			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			//samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.f;
			samplerInfo.minLod = 0.f;
			samplerInfo.maxLod = 1.f;
			VK_ASSERT(vkCreateSampler(renderContext.m_LogicDevice, &samplerInfo, nullptr, &m_Sampler));
		}

		void Texture::CleanTextureData(VkDevice _LogicDevice)
		{
			vkDestroySampler(_LogicDevice, m_Sampler, nullptr);
			m_Sampler = nullptr;
			vkDestroyImageView(_LogicDevice, vk_image.view, nullptr);
#if not USE_VMA
			vkDestroyImage(_LogicDevice, vk_image.image, nullptr);
			vkFreeMemory(_LogicDevice, vk_image.memory, nullptr);
#else
			utils::VMA_DestroyImage(vk_image.image, vk_image.memory);
#endif
		}
	}
}
