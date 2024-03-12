#pragma once
#pragma once
#include <string>
#include <vector>
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>
namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		extern const int FRAMES_IN_FLIGHT;

		struct Texture
		{
		public:
			std::string sPath;
			VkImage tImage = nullptr;
			VkDeviceMemory tImageMem = nullptr;
			VkImageView tImageView = nullptr;
			VkSampler m_Sampler = nullptr;
			Texture(std::string _path)
			{
				sPath = _path;
			}
			void CleanTextureData(VkDevice _LogicDevice)
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
		};
		struct R_CubemapMaterial
		{
			VkShaderModule m_VertShaderModule;
			VkShaderModule m_FragShaderModule;
			VkDescriptorPool m_DescriptorPool = nullptr;
			std::vector<VkDescriptorSetLayout> m_DescLayouts;
			std::vector<VkDescriptorSet> m_DescriptorSet;
		public:
			enum STATE : uint16_t
			{
				UNDEFINED,
				CREATED,
				INICIALITEZ,
				READY,
				DESTROYED
			};
			STATE m_Status = UNDEFINED;
			char _vertPath[64];
			char _fragPath[64];
			Texture* m_Texture;
		public:
			void CreateDescriptorPool(VkDevice _LogicDevice)
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
			void CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout)
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

			void UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers)
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
					TextureDiffuseImage.imageView = m_TextureDiffuse->tImageView;
					if (m_TextureDiffuse->m_Sampler != nullptr)
					{
						TextureDiffuseImage.sampler = m_TextureDiffuse->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureDiffuse->sPath;
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
			void Cleanup(VkDevice _LogicDevice)
			{
				// Delete Material things
				vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
				m_TextureDiffuse->CleanTextureData(_LogicDevice);
				m_TextureDiffuse = nullptr;
			}
		};
	}
}
