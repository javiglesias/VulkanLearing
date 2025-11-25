#include "VKRCubemapMaterial.h"
#include "../../video/VKBackend.h"
#include "../../video/VKRUtils.h"
#include "VKRTexture.h"
#include "../../memory/mem_alloc.h"
#include <array>

namespace VKR
{
	namespace render
	{
		R_CubemapMaterial::R_CubemapMaterial()
		{
		}
		void R_CubemapMaterial::PrepareMaterialToDraw(Texture* _texture, VKBackend* _backend)
		{
			auto renderContext = utils::GetVKContext();
			/// 2 - Crear descriptor pool de materiales(CreateDescPool)
			CreateDescriptorPool(renderContext.m_LogicDevice);

			/// 3 - Crear Descriptor set de material(createMeshDescSet)
			CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_CubemapRender->m_DescSetLayout);

			/// 4 - Crear y transicionar texturas(CreateAndTransImage)
			_backend->LoadCubemapTexture(_texture);
			_backend->CreateAndTransitionImageCubemap(_texture, _backend->m_CommandPool);
			// , VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_CUBE, 6,VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT
		}
		void R_CubemapMaterial::CreateDescriptorPool(VkDevice _LogicDevice)
		{
			if (m_DescriptorPool == nullptr)
			{
				std::array<VkDescriptorPoolSize, 2> descPoolSize{};
				// UBO
				descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descPoolSize[0].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				// Textura
				descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[1].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				VkDescriptorPoolCreateInfo descPoolInfo{};
				descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descPoolInfo.poolSizeCount = static_cast<uint32_t>(descPoolSize.size());
				descPoolInfo.pPoolSizes = descPoolSize.data();
				descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
				if (vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
					exit(-66);
			}
		}
		void R_CubemapMaterial::CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout)
		{
			if (m_DescriptorSet.size() <= 0)
			{
				m_DescLayouts.resize(FRAMES_IN_FLIGHT);
				m_DescLayouts[0] = _DescSetLayout;
				m_DescLayouts[1] = _DescSetLayout;
				VkDescriptorSetAllocateInfo descAllocInfo{};
				descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descAllocInfo.descriptorPool = m_DescriptorPool;
				descAllocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT); // SwapchainImages.size()
				descAllocInfo.pSetLayouts = m_DescLayouts.data();
				m_DescriptorSet.resize(FRAMES_IN_FLIGHT);
				if (vkAllocateDescriptorSets(_LogicDevice, &descAllocInfo, m_DescriptorSet.data()) != VK_SUCCESS)
					exit(-67);
			}
		}
		void R_CubemapMaterial::UpdateDescriptorSet(Texture* _texture,VkDevice _LogicDevice, std::array<VkBuffer, 2> _UniformBuffers)
		{
			// Escribimos la info de los descriptors
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = _UniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(CubemapUniformBufferObject); // VK_WHOLE

				std::array<VkWriteDescriptorSet, 2> descriptorsWrite{};
				descriptorsWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[0].dstSet = m_DescriptorSet[i];
				descriptorsWrite[0].dstBinding = 0;
				descriptorsWrite[0].dstArrayElement = 0;
				descriptorsWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorsWrite[0].descriptorCount = 1;
				descriptorsWrite[0].pBufferInfo = &bufferInfo;
				descriptorsWrite[0].pImageInfo = nullptr;
				descriptorsWrite[0].pTexelBufferView = nullptr;
				// Texturas
				// Diffuse
				VkDescriptorImageInfo TextureDiffuseImage{};
				TextureDiffuseImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				TextureDiffuseImage.imageView = _texture->vk_image.view;
				if (_texture->m_Sampler != nullptr)
				{
					TextureDiffuseImage.sampler = _texture->m_Sampler;
				}
				else
					printf("Invalid Sampler for frame\n");

				descriptorsWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[1].dstSet = m_DescriptorSet[i];
				descriptorsWrite[1].dstBinding = 1;
				descriptorsWrite[1].dstArrayElement = 0;
				descriptorsWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorsWrite[1].descriptorCount = 1;
				descriptorsWrite[1].pBufferInfo = nullptr;
				descriptorsWrite[1].pImageInfo = &TextureDiffuseImage;
				descriptorsWrite[1].pTexelBufferView = nullptr;

				vkUpdateDescriptorSets(_LogicDevice, static_cast<uint32_t>(descriptorsWrite.size()), descriptorsWrite.data(), 0, nullptr);
			}
		}
		void R_CubemapMaterial::Cleanup(VkDevice _LogicDevice)
		{
			// Delete Material things
			vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
		}
	}
}
