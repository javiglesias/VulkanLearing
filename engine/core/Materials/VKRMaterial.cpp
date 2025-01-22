#include "VKRMaterial.h"
#include "VKRTexture.h"
#include "../../perfmon/Custom.h"
#ifndef WIN32
#include <signal.h>
#endif
namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;

		void sMaterialPipeline::_buildPipeline()
		{
			VkDevice m_LogicDevice = GetVKContext().m_LogicDevice;
			VkPipelineShaderStageCreateInfo shaderStages[2] = {};
			VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
			VkPipelineDynamicStateCreateInfo dynamicState{};
			VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
			VkPipelineViewportStateCreateInfo viewportState{
				VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO
			};
			std::vector<VkDynamicState> dynamicStates = {
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkViewport* viewport = new VkViewport();
			VkRect2D* scissor = new VkRect2D();
			
			VkPipelineRasterizationStateCreateInfo rasterizer{};
			VkPipelineMultisampleStateCreateInfo multisampling{};
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			VkPipelineColorBlendStateCreateInfo colorBlending{};
			VkPipelineColorBlendAttachmentState colorBlendAttachment{};
			VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
			VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
			VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
			
			VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;
			VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			Shader* vertShader;
			Shader* fragShader;
			Shader* computeShader = new Shader("engine/shaders/Standard.comp", 5);
			Shader* vert_finded = find_shader("engine/shaders/Standard.vert");
			if(!vert_finded)
			{
				vertShader = new Shader("engine/shaders/Standard.vert", 0);
				add_shader_to_list(vertShader);
			}
			else
			{
				vertShader = vert_finded; 
			}
			vertShader->ConfigureShader(m_LogicDevice, VK_SHADER_STAGE_VERTEX_BIT, &vertShaderStageInfo);
			shaderStages[0] = vertShaderStageInfo;
			
			Shader* frag_finded = find_shader("engine/shaders/Standard.frag");
			if(!frag_finded)
			{
				fragShader = new Shader("engine/shaders/Standard.frag", 0);
				add_shader_to_list(fragShader);
			}
			else
			{
				fragShader = frag_finded; 
			}
			fragShader->ConfigureShader(m_LogicDevice, VK_SHADER_STAGE_FRAGMENT_BIT, &fragShaderStageInfo);
			shaderStages[1] = fragShaderStageInfo;

			computeShader->ConfigureShader(m_LogicDevice, VK_SHADER_STAGE_COMPUTE_BIT, &computeShaderStageInfo);

			/// Color Blending
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_FALSE;
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;
			/// Dynamic State (stados que permiten ser cambiados sin re-crear toda la pipeline)
			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicState.pDynamicStates = dynamicStates.data();

			/// Vertex Input (los datos que le pasamos al shader per-vertex o per-instance)
			vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<unsigned int>(m_AttributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &m_BindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = m_AttributeDescriptions.data();

			/// Definimos la geometria que vamos a pintar
			inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;
			/// Definimos el Viewport de la app
			viewport->x = 0.f;
			viewport->y = 0.f;
			viewport->width = WIN_WIDTH;
			viewport->height = WIN_HEIGHT;
			viewport->minDepth = 0.0f;
			viewport->maxDepth = 1.0f;
			/// definamos el Scissor Rect de la app
			scissor->offset = { 0, 0 };
			scissor->extent.height = 800;
			scissor->extent.width = 600;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;
			viewportState.pViewports = viewport;
			viewportState.pScissors = scissor;

			/*Si no estuvieramos en modo Dynamico, necesitariamos establecer la
			informacion de creacion del ViewportState y los Vewport punteros.*/

			// El rasterizador convierte los vertices en fragmentos para darles color
			rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.f;
			rasterizer.cullMode = cullMode;
			rasterizer.frontFace = frontFace;
			rasterizer.depthBiasEnable = VK_FALSE;
			// Multisampling para evitar los bordes de sierra (anti-aliasing).
			multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.f;
			multisampling.pSampleMask = nullptr;
			multisampling.alphaToCoverageEnable = VK_FALSE;
			multisampling.alphaToOneEnable = VK_FALSE;
			// Depth and stencil
			depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthTestEnable = VK_TRUE;
			depthStencil.depthWriteEnable = VK_TRUE;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f;
			depthStencil.maxDepthBounds = 1.0f;
			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {};
			depthStencil.back.compareOp = VK_COMPARE_OP_ALWAYS;

			// estructura UBO
			VkDescriptorSetLayoutBinding uboLayoutBinding{};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			uboLayoutBinding.pImmutableSamplers = nullptr;
			// Texturas
			VkDescriptorSetLayoutBinding texturesLayoutBinding{};
			texturesLayoutBinding.binding = 1;
			texturesLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			texturesLayoutBinding.descriptorCount = 8;
			texturesLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			texturesLayoutBinding.pImmutableSamplers = nullptr;

			// estructura Dynamic Uniforms
			VkDescriptorSetLayoutBinding dynOLayoutBinding{};
			dynOLayoutBinding.binding = 2;
			dynOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			dynOLayoutBinding.descriptorCount = 1;
			dynOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			dynOLayoutBinding.pImmutableSamplers = nullptr;

			// estructura Directional Light
			VkDescriptorSetLayoutBinding dirLightLayoutBinding{};
			dirLightLayoutBinding.binding = 3;
			dirLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			dirLightLayoutBinding.descriptorCount = 1;
			dirLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			dirLightLayoutBinding.pImmutableSamplers = nullptr;
			// estructura Dynamic Uniforms
			VkDescriptorSetLayoutBinding pointLightLayoutBinding{};
			pointLightLayoutBinding.binding = 4;
			pointLightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			pointLightLayoutBinding.descriptorCount = 1;
			pointLightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			pointLightLayoutBinding.pImmutableSamplers = nullptr;
			// estructura Light Uniforms
			VkDescriptorSetLayoutBinding linOLayoutBinding{};
			linOLayoutBinding.binding = 5;
			linOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			linOLayoutBinding.descriptorCount = 2;
			linOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			linOLayoutBinding.pImmutableSamplers = nullptr;

			std::array<VkDescriptorSetLayoutBinding, 6> ShaderBindings = {
				uboLayoutBinding,
				texturesLayoutBinding,
				dynOLayoutBinding,
				dirLightLayoutBinding,
				pointLightLayoutBinding,
				linOLayoutBinding
			};
			VkDescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			layoutInfo.bindingCount = static_cast<uint32_t>(ShaderBindings.size());
			layoutInfo.pBindings = ShaderBindings.data();
			if (vkCreateDescriptorSetLayout(m_LogicDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
				exit(-99);

			/// Pipeline Layout
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
			pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCreateInfo.setLayoutCount = 1;
			pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
			pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
			if (vkCreatePipelineLayout(m_LogicDevice, &pipelineLayoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
			// Creamos la Pipeline para pintar objs
			VkGraphicsPipelineCreateInfo pipelineInfoCreateInfo{};
			pipelineInfoCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfoCreateInfo.stageCount = 2;
			pipelineInfoCreateInfo.pStages = shaderStages;
			pipelineInfoCreateInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfoCreateInfo.pInputAssemblyState = &inputAssembly;
			pipelineInfoCreateInfo.pViewportState = &viewportState;
			pipelineInfoCreateInfo.pRasterizationState = &rasterizer;
			pipelineInfoCreateInfo.pMultisampleState = &multisampling;
			pipelineInfoCreateInfo.pDepthStencilState = &depthStencil;
			pipelineInfoCreateInfo.pColorBlendState = &colorBlending;
			pipelineInfoCreateInfo.pDynamicState = &dynamicState;
			pipelineInfoCreateInfo.layout = layout;
			pipelineInfoCreateInfo.renderPass = GetVKContext().m_RenderPass->pass;
			pipelineInfoCreateInfo.subpass = 0;
			pipelineInfoCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
			pipelineInfoCreateInfo.basePipelineIndex = -1;

			if (vkCreateGraphicsPipelines(m_LogicDevice, VK_NULL_HANDLE, 1, &pipelineInfoCreateInfo,
				nullptr, &pipeline) != VK_SUCCESS)
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif

			// COMPUTE PIPELINE
			VkComputePipelineCreateInfo compute_pipeInfo {};
			compute_pipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			compute_pipeInfo.layout = compute_layout;
			compute_pipeInfo.stage = computeShaderStageInfo;
			if (vkCreateComputePipelines(m_LogicDevice, VK_NULL_HANDLE, 1, &compute_pipeInfo, 
					nullptr, &compute)  != VK_SUCCESS)
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
			VkPipelineLayoutCreateInfo compute_pipelineLayout{};
			compute_pipelineLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			compute_pipelineLayout.setLayoutCount = 1;
			compute_pipelineLayout.pSetLayouts = &compute_descriptorSetLayout;
			if (vkCreatePipelineLayout(m_LogicDevice, &compute_pipelineLayout, nullptr, &compute_layout) 
					!= VK_SUCCESS)
				#ifdef WIN32
					__debugbreak();
				#else
					raise(SIGTRAP);
				#endif
		}

		void R_Material::PrepareMaterialToDraw(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			PERF_INIT("CREATE_DESC_POOL")
			/// 2 - Crear descriptor pool de materiales(CreateDescPool)
			CreateDescriptor(renderContext.m_LogicDevice);
			PERF_END("CREATE_DESC_POOL")

			/// 3 - Crear Descriptor set de material(createMeshDescSet)
			/*PERF_INIT("CREATE_MESH_DESC")
			CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_GraphicsRender->m_DescSetLayout);
			PERF_END("CREATE_MESH_DESC")*/

			PERF_INIT("CREATE_TEXTURES")
			/// 4 - Crear y transicionar texturas(CreateAndTransImage)
			for (int t = 0; t < 7; t++)
			{
				if (textures[t])
				{
					textures[t]->LoadTexture();
					if(textures[t]->m_Mipmaps > 0)
						textures[t]->CreateAndTransitionImage(_backend->m_CommandPool);
					else
						textures[t]->CreateAndTransitionImageNoMipMaps(_backend->m_CommandPool);
				}
			}
			// Shadow Texture
			textures[7]->tImage = _backend->m_ShadowTexture->tImage;
			textures[7]->tImageView = _backend->m_ShadowTexture->tImageView;
			textures[7]->tImageMem = _backend->m_ShadowTexture->tImageMem;
			textures[7]->m_Sampler = _backend->m_ShadowTexture->m_Sampler;
			PERF_END("CREATE_TEXTURES")
		}
		
		void R_Material::CreateDescriptor(VkDevice _LogicDevice)
		{
			VkDescriptorSetLayout descriptorLayouts[2];
			std::array<VkDescriptorPoolSize, 4> descPoolSize{};
			// UBO
			descPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descPoolSize[0].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			// Textura BaseColor
			/*for (int i = 0; i < 8; i++)
			{*/
				descPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descPoolSize[1].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			//}

			// Model Matrix
			descPoolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descPoolSize[2].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			/*for (int i = 0; i < 4; i ++)
			{*/
				descPoolSize[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descPoolSize[3].descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			//}

			VkDescriptorPoolCreateInfo descPoolInfo{};
			descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolInfo.poolSizeCount = static_cast<uint32_t>(descPoolSize.size());
			descPoolInfo.pPoolSizes = descPoolSize.data();
			descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
			if (vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &material->descriptorPool) != VK_SUCCESS)
				exit(-66);
			descriptorLayouts[0] = material->pipeline.descriptorSetLayout;
			descriptorLayouts[1] = material->pipeline.descriptorSetLayout;
			VkDescriptorSetAllocateInfo descAllocInfo{};
			descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descAllocInfo.descriptorPool = material->descriptorPool;
			descAllocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT); // SwapchainImages.size()
			descAllocInfo.pSetLayouts = descriptorLayouts;
			material->materialSets.resize(FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(_LogicDevice, &descAllocInfo, material->materialSets.data()) != VK_SUCCESS)
				exit(-67);
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

				std::array<VkWriteDescriptorSet, 14> descriptorsWrite{};
				descriptorsWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[0].dstSet = material->materialSets[i];
				descriptorsWrite[0].dstBinding = 0;
				descriptorsWrite[0].dstArrayElement = 0;
				descriptorsWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorsWrite[0].descriptorCount = 1;
				descriptorsWrite[0].pBufferInfo = &bufferInfo;
				descriptorsWrite[0].pImageInfo = nullptr;
				descriptorsWrite[0].pTexelBufferView = nullptr;
				// Texturas
				VkDescriptorImageInfo Textureimage[8] {};
				for (int t = 0; t < 8; t++ )
				{
					// BaseColor
					Textureimage[t].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					Textureimage[t].imageView = textures[t]->tImageView;
					if (textures[t]->m_Sampler != nullptr)
					{
						Textureimage[t].sampler = textures[t]->m_Sampler;
					}
					else
					{
						printf("Invalid Sampler for frame\n");
					}
					descriptorsWrite[1 + t].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[1 + t].dstSet = material->materialSets[i];
					descriptorsWrite[1 + t].dstBinding = 1;
					descriptorsWrite[1 + t].dstArrayElement = t;
					descriptorsWrite[1 + t].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorsWrite[1 + t].descriptorCount = 1;
					descriptorsWrite[1 + t].pBufferInfo = nullptr;
					descriptorsWrite[1 + t].pImageInfo = &Textureimage[t];
					descriptorsWrite[1 + t].pTexelBufferView = nullptr;
				}

				// Dynamic
				VkDescriptorBufferInfo dynBufferInfo{};
				dynBufferInfo.buffer = _DynamicBuffers[i];
				dynBufferInfo.offset = 0;
				dynBufferInfo.range = sizeof(DynamicBufferObject); // VK_WHOLE

				descriptorsWrite[9].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[9].dstSet = material->materialSets[i];
				descriptorsWrite[9].dstBinding = 2;
				descriptorsWrite[9].dstArrayElement = 0;
				descriptorsWrite[9].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorsWrite[9].descriptorCount = 1;
				descriptorsWrite[9].pBufferInfo = &dynBufferInfo;
				descriptorsWrite[9].pImageInfo = nullptr;
				descriptorsWrite[9].pTexelBufferView = nullptr;

				// Directional Light
				VkDescriptorBufferInfo dirLightBufferInfo{};
				dirLightBufferInfo.buffer = _LightsBuffers[i];
				dirLightBufferInfo.offset = 0;
				dirLightBufferInfo.range = sizeof(LightBufferObject); // VK_WHOLE
				descriptorsWrite[10].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[10].dstSet = material->materialSets[i];
				descriptorsWrite[10].dstBinding = 3;
				descriptorsWrite[10].dstArrayElement = 0;
				descriptorsWrite[10].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorsWrite[10].descriptorCount = 1;
				descriptorsWrite[10].pBufferInfo = &dirLightBufferInfo;
				descriptorsWrite[10].pImageInfo = nullptr;
				descriptorsWrite[10].pTexelBufferView = nullptr;

				// Point Light
				VkDescriptorBufferInfo pointtLightBufferInfo{};
				pointtLightBufferInfo.buffer = _LightsBuffers[i];
				pointtLightBufferInfo.offset = 0;
				pointtLightBufferInfo.range = sizeof(LightBufferObject); // VK_WHOLE
				descriptorsWrite[11].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite[11].dstSet = material->materialSets[i];
				descriptorsWrite[11].dstBinding = 4;
				descriptorsWrite[11].dstArrayElement = 0;
				descriptorsWrite[11].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				descriptorsWrite[11].descriptorCount = 1;
				descriptorsWrite[11].pBufferInfo = &pointtLightBufferInfo;
				descriptorsWrite[11].pImageInfo = nullptr;
				descriptorsWrite[11].pTexelBufferView = nullptr;

				// Dynamic
				VkDescriptorBufferInfo lightBufferInfo{};
				lightBufferInfo.buffer = _LightsBuffers[i];
				lightBufferInfo.offset = 0;
				lightBufferInfo.range = sizeof(LightBufferObject); // VK_WHOLE
				int start = 12;
				for(int j = start; j < 14; j++) // 4 lights
				{
					descriptorsWrite[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorsWrite[j].dstSet = material->materialSets[i];
					descriptorsWrite[j].dstBinding = 5;
					descriptorsWrite[j].dstArrayElement = j - start;
					descriptorsWrite[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					descriptorsWrite[j].descriptorCount = 1;
					descriptorsWrite[j].pBufferInfo = &lightBufferInfo;
					descriptorsWrite[j].pImageInfo = nullptr;
					descriptorsWrite[j].pTexelBufferView = nullptr;
				}
				vkUpdateDescriptorSets(_LogicDevice, (uint32_t)descriptorsWrite.size(), descriptorsWrite.data(), 0, nullptr);
			}
		}

		void R_Material::Cleanup(VkDevice _LogicDevice)
		{
			material->Cleanup(_LogicDevice);
			// Delete Material things
			textures[0]->CleanTextureData(_LogicDevice);
			textures[0] = nullptr;
			textures[1]->CleanTextureData(_LogicDevice);
			textures[1] = nullptr;
			textures[2]->CleanTextureData(_LogicDevice);
			textures[2] = nullptr;
			textures[3]->CleanTextureData(_LogicDevice);
			textures[3] = nullptr;
			textures[4]->CleanTextureData(_LogicDevice);
			textures[4] = nullptr;
			textures[5]->CleanTextureData(_LogicDevice);
			textures[5] = nullptr;
			textures[6]->CleanTextureData(_LogicDevice);
			textures[6] = nullptr;
		}

		void MaterialInstance::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyDescriptorPool(_LogicDevice, descriptorPool, nullptr);
			pipeline.Cleanup(_LogicDevice);
		}

		void sMaterialPipeline::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyDescriptorSetLayout(_LogicDevice, descriptorSetLayout, nullptr);
			vkDestroyPipelineLayout(_LogicDevice, layout, nullptr);
			vkDestroyPipeline(_LogicDevice, pipeline, nullptr);
		}
	}
}
