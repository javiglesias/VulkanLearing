#include "VKRMaterial.h"
#include "../../perfmon/Custom.h"
#include "../../video/GPUParticle.h"
#include "../../video/VKBackend.h"
#include "../../video/VKRUtils.h"
#include "VKRTexture.h"
#ifndef WIN32
#include <signal.h>
#endif
namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;

		void sMaterialPipeline::_addBind(uint8_t _nbind, VkDescriptorType _type,
			VkShaderStageFlags _stage, uint8_t _ndescriptors)
		{
			VkDescriptorSetLayoutBinding binding{};
			binding.binding = _nbind;
			binding.descriptorType = _type;
			binding.descriptorCount = _ndescriptors;
			binding.stageFlags = _stage;
			binding.pImmutableSamplers = nullptr;
			ShaderBindings.push_back(binding);
		}

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
			vertShader->LoadShader();
			vertShader->ConfigureShader(m_LogicDevice, VK_SHADER_STAGE_VERTEX_BIT, &vertShaderStageInfo);
			shaderStages[0] = vertShaderStageInfo;
			
			Shader* fragShader;
			Shader* frag_finded = find_shader("engine/shaders/Standard.frag");
			if(!frag_finded)
			{
				fragShader = new Shader("engine/shaders/Standard.frag", 4);
				add_shader_to_list(fragShader);
			}
			else
			{
				fragShader = frag_finded; 
			}
			fragShader->LoadShader();
			fragShader->ConfigureShader(m_LogicDevice, VK_SHADER_STAGE_FRAGMENT_BIT, &fragShaderStageInfo);
			shaderStages[1] = fragShaderStageInfo;

			Shader* computeShader = new Shader("engine/shaders/Standard.comp", 5);
			computeShader->LoadShader();
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
			_addBind(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
			// Texturas
			_addBind(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 8);
			// estructura Dynamic Uniforms
			_addBind(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT);
			// estructura Directional Light
			_addBind(3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
			// estructura Dynamic Uniforms
			_addBind(4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT);
			// estructura Light Uniforms
			_addBind(5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 2);

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
			VkDescriptorSetLayoutBinding computeLayoutBinding{};
			computeLayoutBinding.binding = 0;
			computeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			computeLayoutBinding.descriptorCount = 1;
			computeLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
			computeLayoutBinding.pImmutableSamplers = nullptr;
			VkDescriptorSetLayoutCreateInfo computeLayoutInfo{};
			computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			computeLayoutInfo.bindingCount = 1;
			computeLayoutInfo.pBindings = &computeLayoutBinding;
			if (vkCreateDescriptorSetLayout(m_LogicDevice, &computeLayoutInfo, nullptr, &compute_descriptorSetLayout) != VK_SUCCESS)
				exit(-99);
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
			VkComputePipelineCreateInfo compute_pipeInfo {};
			compute_pipeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			compute_pipeInfo.layout = compute_layout;
			compute_pipeInfo.stage = computeShaderStageInfo;
			compute_pipeInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
			if (vkCreateComputePipelines(m_LogicDevice, VK_NULL_HANDLE, 1, &compute_pipeInfo, 
					nullptr, &compute)  != VK_SUCCESS)
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
			textures[7]->vk_image = _backend->m_ShadowTexture->vk_image;
			textures[7]->m_Sampler = _backend->m_ShadowTexture->m_Sampler;
			PERF_END("CREATE_TEXTURES")
		}
		
		void R_Material::CreateDescriptor(VkDevice _LogicDevice)
		{
			std::vector<VkDescriptorPoolSize> descPoolSize;
			// UBO
			VkDescriptorPoolSize temp;
			temp.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			temp.descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			descPoolSize.push_back(temp);
			// Textura BaseColor
			/*for (int i = 0; i < 8; i++)
			{*/
				temp.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				temp.descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				descPoolSize.push_back(temp);
			//}

			// Model Matrix
			temp.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			temp.descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			descPoolSize.push_back(temp);
			/*for (int i = 0; i < 4; i ++)
			{*/
				temp.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
				temp.descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
				descPoolSize.push_back(temp);
			//}

			VkDescriptorPoolCreateInfo descPoolInfo{};
			VkDescriptorSetLayout descriptorLayouts[2];
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


			#pragma region COMPUTE_DESC
			std::vector<VkDescriptorPoolSize> compute_descPoolSize;
			VkDescriptorPoolSize compute_descriptor_poolSize;
			compute_descriptor_poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			compute_descriptor_poolSize.descriptorCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT);
			compute_descPoolSize.push_back(compute_descriptor_poolSize);

			VkDescriptorPoolCreateInfo compute_descPoolInfo{};
			VkDescriptorSetLayout compute_descriptorLayouts[2];
			compute_descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			compute_descPoolInfo.poolSizeCount = static_cast<uint32_t>(compute_descPoolSize.size());
			compute_descPoolInfo.pPoolSizes = compute_descPoolSize.data();
			compute_descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
			if (vkCreateDescriptorPool(_LogicDevice, &compute_descPoolInfo, nullptr, &material->compute_descriptor_pool) != VK_SUCCESS)
				exit(-66);
			compute_descriptorLayouts[0] = material->pipeline.compute_descriptorSetLayout;
			compute_descriptorLayouts[1] = material->pipeline.compute_descriptorSetLayout;
			VkDescriptorSetAllocateInfo compute_descAllocInfo{};
			compute_descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			compute_descAllocInfo.descriptorPool = material->compute_descriptor_pool;
			compute_descAllocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT); // SwapchainImages.size()
			compute_descAllocInfo.pSetLayouts = compute_descriptorLayouts;
			material->compute_descriptors_sets.resize(FRAMES_IN_FLIGHT);
			if (vkAllocateDescriptorSets(_LogicDevice, &compute_descAllocInfo, material->compute_descriptors_sets.data()) != VK_SUCCESS)
				exit(-67);
			#pragma endregion
		}

		void R_Material::PrepareDescriptorWrite(int16_t _setDst, uint32_t _bind,
			VkDescriptorType _type, VkBuffer _buffer, VkDeviceSize _range, uint32_t _arrayElement,
			VkDeviceSize _offset, uint32_t _count)
		{
			VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();;
			bufferInfo->buffer = _buffer;
			bufferInfo->offset = _offset;
			bufferInfo->range = _range; // VK_WHOLE

			VkWriteDescriptorSet descriptorsWrite{};
			descriptorsWrite.pBufferInfo = bufferInfo;
			descriptorsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite.dstSet = material->materialSets[_setDst];
			descriptorsWrite.dstBinding = _bind;
			descriptorsWrite.dstArrayElement = _arrayElement;
			descriptorsWrite.descriptorType = _type;
			descriptorsWrite.descriptorCount = _count;
			descriptorsWrite.pTexelBufferView = nullptr;
			m_DescriptorsWrite.push_back(descriptorsWrite);
		}

		void R_Material::PrepareDescriptorWrite(int16_t _setDst, uint32_t _bind, VkDescriptorType _type,
			VkImageView _imageView, VkSampler _sampler, uint32_t _arrayElement, uint32_t _count)
		{
			VkDescriptorImageInfo* Textureimage = new VkDescriptorImageInfo();
			Textureimage->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			Textureimage->imageView = _imageView;
			if (_sampler != nullptr)
				Textureimage->sampler = _sampler;
			else
				printf("Invalid Sampler for frame\n");

			VkWriteDescriptorSet descriptorsWrite{};
			descriptorsWrite.pImageInfo = Textureimage;
			descriptorsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorsWrite.dstSet = material->materialSets[_setDst];
			descriptorsWrite.dstBinding = _bind;
			descriptorsWrite.dstArrayElement = _arrayElement;
			descriptorsWrite.descriptorType = _type;
			descriptorsWrite.descriptorCount = _count;
			descriptorsWrite.pTexelBufferView = nullptr;
			m_DescriptorsWrite.push_back(descriptorsWrite);
		}

		void R_Material::CreateDescriptorPool(
				VkDevice _LogicDevice
				, std::vector<VkDescriptorPoolSize> _desc_pool_size
				, VkDescriptorPool _desc_pool
				, VkDescriptorSetLayout _desc_set_layout
				, VkDescriptorSet* _desc_sets)
		{
			VkDescriptorPoolCreateInfo descPoolInfo{};
			VkDescriptorSetLayout descriptorLayouts[2];
			descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolInfo.poolSizeCount = static_cast<uint32_t>(_desc_pool_size.size());
			descPoolInfo.pPoolSizes = _desc_pool_size.data();
			descPoolInfo.maxSets = FRAMES_IN_FLIGHT;
			if (vkCreateDescriptorPool(_LogicDevice, &descPoolInfo, nullptr, &_desc_pool) != VK_SUCCESS)
				exit(-66);
			descriptorLayouts[0] = _desc_set_layout;
			descriptorLayouts[1] = _desc_set_layout;
			VkDescriptorSetAllocateInfo descAllocInfo{};
			descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descAllocInfo.descriptorPool = _desc_pool;
			descAllocInfo.descriptorSetCount = static_cast<uint32_t>(FRAMES_IN_FLIGHT); // SwapchainImages.size()
			descAllocInfo.pSetLayouts = descriptorLayouts;
			_desc_sets = (VkDescriptorSet*)malloc(FRAMES_IN_FLIGHT * sizeof(VkDescriptorSet));
			if (vkAllocateDescriptorSets(_LogicDevice, &descAllocInfo, _desc_sets) != VK_SUCCESS)
				exit(-67);
		}

		void R_Material::UpdateDescriptorSet(VkDevice _LogicDevice, std::vector<VkBuffer> _UniformBuffers, std::vector<VkBuffer> _DynamicBuffers,
		                                     std::vector<VkBuffer> _LightsBuffers, std::vector<VkBuffer> _ComputeBuffers)
		{
			// Escribimos la info de los descriptors
			for (int8_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				PrepareDescriptorWrite(i, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, _UniformBuffers[i], sizeof(UniformBufferObject));
				// Texturas
				for (int8_t t = 0; t < 8; t++ )
				{
					PrepareDescriptorWrite(i, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
							, textures[t]->vk_image.view, textures[t]->m_Sampler, t);
				}
				// Dynamic
				PrepareDescriptorWrite(i, 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _DynamicBuffers[i], sizeof(DynamicBufferObject));
				// Directional Light
				PrepareDescriptorWrite(i, 3, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _LightsBuffers[i], sizeof(LightBufferObject));
				// Point Light
				PrepareDescriptorWrite(i, 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _LightsBuffers[i], sizeof(LightBufferObject));
				// Dynamic
				int start = 12;
				for(int j = start; j < 14; j++) // 4 lights
				{
					PrepareDescriptorWrite(i, 5, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _LightsBuffers[i], sizeof(LightBufferObject), j - start);
				}

				// Compute
				VkDescriptorBufferInfo* bufferInfo = new VkDescriptorBufferInfo();
				bufferInfo->buffer = _ComputeBuffers[i];
				bufferInfo->offset = 0;
				bufferInfo->range = sizeof(GPU::Particle); // VK_WHOLE

				VkWriteDescriptorSet descriptorsWrite{};
				descriptorsWrite.pBufferInfo = bufferInfo;
				descriptorsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorsWrite.dstSet = material->compute_descriptors_sets[i];
				descriptorsWrite.dstBinding = 0;
				descriptorsWrite.dstArrayElement = 0;
				descriptorsWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
				descriptorsWrite.descriptorCount = 1;
				descriptorsWrite.pTexelBufferView = nullptr;
				m_DescriptorsWrite.push_back(descriptorsWrite);
				/*PrepareDescriptorWrite(i, 6, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _ComputeBuffers[i], sizeof(GPU::Particle));*/

				vkUpdateDescriptorSets(_LogicDevice, m_DescriptorsWrite.size(), m_DescriptorsWrite.data(), 0, nullptr);
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
