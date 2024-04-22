#include "VKRCubemapMaterial.h"
#include "Types.h"
#include <array>

namespace VKR
{
	namespace render
	{
		R_CubemapMaterial::R_CubemapMaterial(std::string _path)
		{
			// cubemaps_skybox
			auto tempName = _path + "_0";
			/*unsigned char* data = stbi_load(tempName.c_str(), &width, &height, &nrChannels, 0);
			if (data)
			{
			}*/
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
		void R_CubemapMaterial::UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers)
		{
			// Escribimos la info de los descriptors
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = _UniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject); // VK_WHOLE

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
				TextureDiffuseImage.imageView = m_Texture->tImageView;
				if (m_Texture->m_Sampler != nullptr)
				{
					TextureDiffuseImage.sampler = m_Texture->m_Sampler;
				}
				else
				{
					printf("Invalid Sampler for frame %ld\n", i);
					continue;
				}
				g_ConsoleMSG += m_Texture->sPath;
				g_ConsoleMSG += '\n';
				descriptorsWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[1].dstSet = m_DescriptorSet[i];
				descriptorsWrite[1].dstBinding = 1;
				descriptorsWrite[1].dstArrayElement = 0;
				descriptorsWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorsWrite[1].descriptorCount = 1;
				descriptorsWrite[1].pBufferInfo = nullptr;
				descriptorsWrite[1].pImageInfo = &TextureDiffuseImage;
				descriptorsWrite[1].pTexelBufferView = nullptr;
				vkUpdateDescriptorSets(_LogicDevice, descriptorsWrite.size(), descriptorsWrite.data(), 0, nullptr);
			}
		}
		void R_CubemapMaterial::Cleanup(VkDevice _LogicDevice)
		{
			// Delete Material things
			vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
			m_Texture->CleanTextureData(_LogicDevice);
			m_Texture = nullptr;
		}
	}
}
