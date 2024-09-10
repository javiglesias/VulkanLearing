#include "VKRMaterial.h"
#include "../../video/VKBackend.h"

namespace VKR
{
	namespace render
	{
		void R_Material::PrepareMaterialToDraw(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			/// 2 - Crear descriptor pool de materiales(CreateDescPool)
			CreateDescriptorPool(renderContext.m_LogicDevice);

			/// 3 - Crear Descriptor set de material(createMeshDescSet)
			CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_GraphicsRender->m_DescSetLayout);

			/// 4 - Crear y transicionar texturas(CreateAndTransImage)
			m_TextureDiffuse->LoadTexture();
			m_TextureSpecular->LoadTexture();
			m_TextureAmbient->LoadTexture();
			/*m_TextureEmissive->LoadTexture();
			m_TextureOcclusion->LoadTexture();
			m_TextureMetallicRoughness->LoadTexture();
			m_TextureNormal->LoadTexture();*/

			m_TextureDiffuse->CreateAndTransitionImage(_backend->m_CommandPool);
			m_TextureSpecular->CreateAndTransitionImage(_backend->m_CommandPool);
			m_TextureAmbient->CreateAndTransitionImage(_backend->m_CommandPool);
			/*m_TextureEmissive->CreateAndTransitionImage(_backend->m_CommandPool);
			m_TextureOcclusion->CreateAndTransitionImage(_backend->m_CommandPool);
			m_TextureMetallicRoughness->CreateAndTransitionImage(_backend->m_CommandPool);
			m_TextureNormal->CreateAndTransitionImage(_backend->m_CommandPool);*/
		}

		void R_Material::CreateDescriptorPool(VkDevice _LogicDevice)
		{
			if (m_DescriptorPool == nullptr)
			{
				std::array<VkDescriptorPoolSize, 10> descPoolSize{};
				// UBO
				descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descPoolSize[0].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				// Textura diffuse
				descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[1].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				// Textura Specular
				descPoolSize[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[2].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				// Textura Ambient
				descPoolSize[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[3].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				// Model Matrix
				descPoolSize[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[4].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				// Textura Shadow
				descPoolSize[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[5].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				descPoolSize[6].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[6].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				descPoolSize[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[7].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				descPoolSize[8].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[8].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				descPoolSize[9].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[9].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				//// Textura Emissive
				//descPoolSize[10].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//descPoolSize[10].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				//// Textura Occlusion
				//descPoolSize[11].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//descPoolSize[11].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				//// Textura Metallic Rough
				//descPoolSize[12].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//descPoolSize[12].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				//// Textura Metallic Rough
				//descPoolSize[13].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				//descPoolSize[13].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);

				VkDescriptorPoolCreateInfo descPoolInfo{};
				descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
				descPoolInfo.poolSizeCount = static_cast<uint32_t>(descPoolSize.size());
				descPoolInfo.pPoolSizes = descPoolSize.data();
				descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
				if (vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
					exit(-66);
			}
		}

		void R_Material::CreateMeshDescriptorSet(VkDevice _LogicDevice, VkDescriptorSetLayout _DescSetLayout)
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

		void R_Material::UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers,
															std::vector<VkBuffer> _LightsBuffers)
		{
			// Escribimos la info de los descriptors
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = _UniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject); // VK_WHOLE

				std::array<VkWriteDescriptorSet, 10> descriptorsWrite{};
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
				g_ConsoleMSG += m_TextureDiffuse->m_Path;
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
				//Specular
				VkDescriptorImageInfo TextureSpecularImage{};
				TextureSpecularImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				TextureSpecularImage.imageView = m_TextureSpecular->tImageView;
				if (m_TextureSpecular->m_Sampler != nullptr)
				{
					TextureSpecularImage.sampler = m_TextureSpecular->m_Sampler;
				}
				else
				{
					printf("Invalid Sampler for frame %ld\n", i);
					continue;
				}
				g_ConsoleMSG += m_TextureSpecular->m_Path;
				g_ConsoleMSG += '\n';
				descriptorsWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[2].dstSet = m_DescriptorSet[i];
				descriptorsWrite[2].dstBinding = 2;
				descriptorsWrite[2].dstArrayElement = 0;
				descriptorsWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorsWrite[2].descriptorCount = 1;
				descriptorsWrite[2].pBufferInfo = nullptr;
				descriptorsWrite[2].pImageInfo = &TextureSpecularImage;
				descriptorsWrite[2].pTexelBufferView = nullptr;
				//Ambient
				VkDescriptorImageInfo TextureAmbientImage{};
				TextureAmbientImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				TextureAmbientImage.imageView = m_TextureAmbient->tImageView;
				if (m_TextureAmbient->m_Sampler != nullptr)
				{
					TextureAmbientImage.sampler = m_TextureAmbient->m_Sampler;
				}
				else
				{
					printf("Invalid Sampler for frame %ld\n", i);
					continue;
				}
				g_ConsoleMSG += m_TextureAmbient->m_Path;
				g_ConsoleMSG += '\n';
				descriptorsWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[3].dstSet = m_DescriptorSet[i];
				descriptorsWrite[3].dstBinding = 3;
				descriptorsWrite[3].dstArrayElement = 0;
				descriptorsWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorsWrite[3].descriptorCount = 1;
				descriptorsWrite[3].pBufferInfo = nullptr;
				descriptorsWrite[3].pImageInfo = &TextureAmbientImage;
				descriptorsWrite[3].pTexelBufferView = nullptr;
				// Dynamic
				VkDescriptorBufferInfo dynBufferInfo{};
				dynBufferInfo.buffer = _DynamicBuffers[i];
				dynBufferInfo.offset = 0;
				dynBufferInfo.range = sizeof(DynamicBufferObject); // VK_WHOLE

				descriptorsWrite[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[4].dstSet = m_DescriptorSet[i];
				descriptorsWrite[4].dstBinding = 4;
				descriptorsWrite[4].dstArrayElement = 0;
				descriptorsWrite[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorsWrite[4].descriptorCount = 1;
				descriptorsWrite[4].pBufferInfo = &dynBufferInfo;
				descriptorsWrite[4].pImageInfo = nullptr;
				descriptorsWrite[4].pTexelBufferView = nullptr;

				//Shadow
				{
					VkDescriptorImageInfo TextureShadowImage{};
					TextureShadowImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					TextureShadowImage.imageView = m_TextureShadowMap->tImageView;
					if (m_TextureShadowMap->m_Sampler != nullptr)
					{
						TextureShadowImage.sampler = m_TextureShadowMap->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureShadowMap->m_Path;
					g_ConsoleMSG += '\n';
					descriptorsWrite[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[5].dstSet = m_DescriptorSet[i];
					descriptorsWrite[5].dstBinding = 5;
					descriptorsWrite[5].dstArrayElement = 0;
					descriptorsWrite[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[5].descriptorCount = 1;
					descriptorsWrite[5].pBufferInfo = nullptr;
					descriptorsWrite[5].pImageInfo = &TextureShadowImage;
					descriptorsWrite[5].pTexelBufferView = nullptr;
				}
				// Dynamic
				VkDescriptorBufferInfo lightBufferInfo{};
				lightBufferInfo.buffer = _LightsBuffers[i];
				lightBufferInfo.offset = 0;
				lightBufferInfo.range = sizeof(LightBufferObject); // VK_WHOLE
				int start = 6, end = 6;
				for(int j = start; j < 10; j++)
				{
					descriptorsWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[j].dstSet = m_DescriptorSet[i];
					descriptorsWrite[j].dstBinding = start;
					descriptorsWrite[j].dstArrayElement = j - start;
					descriptorsWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					descriptorsWrite[j].descriptorCount = 1;
					descriptorsWrite[j].pBufferInfo = &lightBufferInfo;
					descriptorsWrite[j].pImageInfo = nullptr;
					descriptorsWrite[j].pTexelBufferView = nullptr;
					end = j;
				}
#if 0
				//Emissive
				{
					VkDescriptorImageInfo TextureEmissiveImage{};
					TextureEmissiveImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					TextureEmissiveImage.imageView = m_TextureEmissive->tImageView;
					if (m_TextureEmissive->m_Sampler != nullptr)
					{
						TextureEmissiveImage.sampler = m_TextureEmissive->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureEmissive->m_Path;
					g_ConsoleMSG += '\n';
					descriptorsWrite[end].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[end].dstSet = m_DescriptorSet[i];
					descriptorsWrite[end].dstBinding = end;
					descriptorsWrite[end].dstArrayElement = 0;
					descriptorsWrite[end].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[end].descriptorCount = 1;
					descriptorsWrite[end].pBufferInfo = nullptr;
					descriptorsWrite[end].pImageInfo = &TextureEmissiveImage;
					descriptorsWrite[end].pTexelBufferView = nullptr;
				}
				//Occlusion
				{
					VkDescriptorImageInfo TextureOcclusionImage{};
					TextureOcclusionImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					TextureOcclusionImage.imageView = m_TextureOcclusion->tImageView;
					if (m_TextureOcclusion->m_Sampler != nullptr)
					{
						TextureOcclusionImage.sampler = m_TextureOcclusion->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureOcclusion->m_Path;
					g_ConsoleMSG += '\n';
					descriptorsWrite[end+1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[end+1].dstSet = m_DescriptorSet[i];
					descriptorsWrite[end+1].dstBinding = end+1;
					descriptorsWrite[end+1].dstArrayElement = 0;
					descriptorsWrite[end+1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[end+1].descriptorCount = 1;
					descriptorsWrite[end+1].pBufferInfo = nullptr;
					descriptorsWrite[end+1].pImageInfo = &TextureOcclusionImage;
					descriptorsWrite[end+1].pTexelBufferView = nullptr;
				}

				//Metallic
				{
					VkDescriptorImageInfo TextureMetallicImage{};
					TextureMetallicImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					TextureMetallicImage.imageView = m_TextureMetallicRoughness->tImageView;
					if (m_TextureMetallicRoughness->m_Sampler != nullptr)
					{
						TextureMetallicImage.sampler = m_TextureMetallicRoughness->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureMetallicRoughness->m_Path;
					g_ConsoleMSG += '\n';
					descriptorsWrite[end+2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[end+2].dstSet = m_DescriptorSet[i];
					descriptorsWrite[end+2].dstBinding = end+2;
					descriptorsWrite[end+2].dstArrayElement = 0;
					descriptorsWrite[end+2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[end+2].descriptorCount = 1;
					descriptorsWrite[end+2].pBufferInfo = nullptr;
					descriptorsWrite[end+2].pImageInfo = &TextureMetallicImage;
					descriptorsWrite[end+2].pTexelBufferView = nullptr;
				}
				//Normal
				{
					VkDescriptorImageInfo TextureNormalImage{};
					TextureNormalImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					TextureNormalImage.imageView = m_TextureNormal->tImageView;
					if (m_TextureNormal->m_Sampler != nullptr)
					{
						TextureNormalImage.sampler = m_TextureNormal->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame %ld\n", i);
						continue;
					}
					g_ConsoleMSG += m_TextureNormal->m_Path;
					g_ConsoleMSG += '\n';
					descriptorsWrite[end+3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[end+3].dstSet = m_DescriptorSet[i];
					descriptorsWrite[end+3].dstBinding = end+3;
					descriptorsWrite[end+3].dstArrayElement = 0;
					descriptorsWrite[end+3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[end+3].descriptorCount = 1;
					descriptorsWrite[end+3].pBufferInfo = nullptr;
					descriptorsWrite[end+3].pImageInfo = &TextureNormalImage;
					descriptorsWrite[end+3].pTexelBufferView = nullptr;
				}
#endif
				vkUpdateDescriptorSets(_LogicDevice, descriptorsWrite.size(), descriptorsWrite.data(), 0, nullptr);
			}
		}

		void R_Material::Cleanup(VkDevice _LogicDevice)
		{
			// Delete Material things
			vkDestroyDescriptorPool(_LogicDevice, m_DescriptorPool, nullptr);
			m_TextureDiffuse->CleanTextureData(_LogicDevice);
			m_TextureDiffuse = nullptr;
			m_TextureSpecular->CleanTextureData(_LogicDevice);
			m_TextureSpecular = nullptr;
			m_TextureAmbient->CleanTextureData(_LogicDevice);
			m_TextureAmbient = nullptr;
			m_TextureShadowMap->CleanTextureData(_LogicDevice);
			m_TextureShadowMap = nullptr;
			if(m_TextureEmissive)
				m_TextureEmissive->CleanTextureData(_LogicDevice);
			if(m_TextureOcclusion)
				m_TextureOcclusion->CleanTextureData(_LogicDevice);
			if(m_TextureMetallicRoughness)
				m_TextureMetallicRoughness->CleanTextureData(_LogicDevice);
			if(m_TextureNormal)
				m_TextureNormal->CleanTextureData(_LogicDevice);
			m_TextureEmissive = nullptr;
			m_TextureOcclusion = nullptr;
			m_TextureMetallicRoughness = nullptr;
			m_TextureNormal = nullptr;
		}
	}
}
