#include "VKRScene.h"
#include "../perfmon/Custom.h"
#include "../video/Types.h"
#include "Objects/VKRCubemap.h"
#include "Objects/VKRLight.h"
#include "Objects/VKRModel.h"
#include <cstddef>
#include "../filesystem/ResourceManager.h"
#include "../editor/Editor.h"
#include "Materials/VKRTexture.h"
#include "../video/GPUParticle.h"

namespace VKR
{
	namespace render
	{
		void clean_material_list();
		void add_instance_to_list(MaterialInstance* _material);
		MaterialInstance* find_instance(uint8_t _id);

		static Scene g_MainScene;
		Scene& GetVKMainScene()
		{
			return g_MainScene;
		}

		/// Shadow Pass
		void Scene::ShadowPass(VKBackend* _backend, int _CurrentFrame)
		{
			if(m_SceneDirty)
			{
				PrepareScene(_backend);
				m_SceneDirty = false;
			}
			//glm::mat4 shadowProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
			glm::mat4 orthogonalProjMat = glm::ortho<float>(g_DirectionalLight->m_Right, -g_DirectionalLight->m_Right,
									g_DirectionalLight->m_Up, -g_DirectionalLight->m_Up, 
									g_DirectionalLight->m_Depth, -g_DirectionalLight->m_Depth);
			orthogonalProjMat[1][1] *= -1;
			glm::mat4 lightViewMat = glm::lookAt( g_DirectionalLight->m_Pos, g_DirectionalLight->m_Pos + g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
			glm::mat4 lightProjMat = orthogonalProjMat * lightViewMat;
			auto renderContext = GetVKContext();
			// Clear Color
			VkClearValue clearValue;
			clearValue.depthStencil = { 1.0f, 0 };
			// Update Uniform buffers
			ShadowUniformBufferObject ubo{};
			ubo.view = lightViewMat;
			ubo.projection = lightProjMat;
			ubo.depthMVP = lightProjMat * lightViewMat;
			memcpy(_backend->m_ShadowUniformBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderContext.m_ShadowPass->pass;
			renderPassInfo.framebuffer = _backend->m_ShadowFramebuffer;
			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = _backend->m_CurrentExtent;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearValue;
			vkCmdBeginRenderPass(_backend->m_CommandBuffer[_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowRender->m_Pipeline);
			vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
			vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);

			auto dynamicAlignment = sizeof(DynamicBufferObject);
			if (renderContext.m_GpuInfo.minUniformBufferOffsetAlignment > 0)
			{
				dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1) & ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			}
			uint32_t count = 0;
			// Sombras(Depth Pass)
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				DynamicBufferObject dynO{};				
				dynO.model = model->m_ModelMatrix;
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_ShadowDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowRender->m_PipelineLayout,
					0, 1, &_backend->m_ShadowMat->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers
					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer[_CurrentFrame] };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
					// Draw Loop
					if (mesh->m_Indices.size() > 0)
					{
						vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer[_CurrentFrame], 0, VK_INDEX_TYPE_UINT16);
						vkCmdDrawIndexed(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], (uint32_t)mesh->m_Vertices.size(), 1, 0, 0);
					}
					// Flush to make changes visible to the host
				}
				++count;
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = _backend->m_ShadowDynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
			}
			vkCmdEndRenderPass(_backend->m_CommandBuffer[_CurrentFrame]);
		}

		void Scene::GeometryPass(VKBackend* _backend, int _CurrentFrame)
		{

		}

		void Scene::ReloadShaders(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			vkDeviceWaitIdle(renderContext.m_LogicDevice);
			vkDestroyPipeline(renderContext.m_LogicDevice, m_CubemapRender->m_Pipeline, nullptr);
			vkDestroyPipelineLayout(renderContext.m_LogicDevice, m_CubemapRender->m_PipelineLayout, nullptr);
			m_CubemapRender->Initialize(true);
			m_CubemapRender->CreatePipelineLayoutSetup(&_backend->m_CurrentExtent, &_backend->m_Viewport, &_backend->m_Scissor);
			m_CubemapRender->CreatePipelineLayout();
			m_CubemapRender->CreatePipeline(renderContext.m_RenderPass->pass);
			m_CubemapRender->CleanShaderModules();
			PERF_INIT("RELOAD_SHADERS")
			render::clean_material_list();
			for(int m = 0; m < m_CurrentStaticModels; m++)
			{
				for(int mi = 0; mi < m_StaticModels[m]->nMaterials; mi++)
				{
					//m_StaticModels[m]->m_Materials[mi]->material.pipeline.Cleanup(renderContext.m_LogicDevice);
					m_StaticModels[m]->m_Materials[mi]->material->pipeline._buildPipeline();
					//m_StaticModels[m]->m_Materials[mi]->PrepareMaterialToDraw(_backend);
				}
			}
#if 0
			if (m_ShadowRender->m_VertShader->GLSLCompile(true))
			{
				vkDestroyPipeline(renderContext.m_LogicDevice, m_ShadowRender->m_Pipeline, nullptr);
				vkDestroyPipelineLayout(renderContext.m_LogicDevice, m_ShadowRender->m_PipelineLayout, nullptr);
				m_ShadowRender->Initialize(true);
				m_ShadowRender->CreatePipelineLayoutSetup(&_backend->m_CurrentExtent, &_backend->m_Viewport, &_backend->m_Scissor);
				m_ShadowRender->CreatePipelineLayout();
				m_ShadowRender->CreatePipeline(g_context.m_ShadowPass->pass);
				m_ShadowRender->CleanShaderModules();
			}
#endif
			PERF_END("RELOAD_SHADERS")
		}

		void Scene::DrawScene(VKBackend* _backend, int _CurrentFrame)
		{
			auto imageIdx = _backend->BeginFrame(_CurrentFrame);
			g_editor->Loop(this, _backend);
			auto renderContext = GetVKContext();
			// GeometryPass(_backend, _CurrentFrame);
			ShadowPass(_backend, _CurrentFrame);
			vkCmdResetQueryPool(_backend->m_CommandBuffer[_CurrentFrame], _backend->m_PerformanceQuery[_CurrentFrame], 0, static_cast<uint32_t>(g_Timestamps.size()));
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			auto lightDynAlign = sizeof(LightBufferObject);
			lightDynAlign = (lightDynAlign + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(m_CameraFOV), static_cast<float>(g_WindowWidth / g_WindowHeight), zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			glm::mat4 viewMat = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			/// Render Pass
			// Update Uniform buffers
			UniformBufferObject ubo{};
			ubo.view = viewMat;
			ubo.projection = projMat;
			ubo.cameraPosition = m_CameraPos;
			memcpy(_backend->m_Uniform_SBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));
			_backend->BeginRenderPass(_CurrentFrame);
#pragma region GRID
#if 0
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GridRender->m_Pipeline);
			DebugUniformBufferObject gubo{};
			gubo.view = viewMat;
			gubo.projection = projMat;
			memcpy(_backend->m_GridUniformBuffersMapped[_CurrentFrame], &gubo, sizeof(gubo));
#endif
#pragma endregion
#pragma region MODELS
			//vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsRender->m_Pipeline);
			// REFRESH RENDER MODE FUNCTIONS
			//vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
			//vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);
			uint32_t count = 0;
			// Render Pass
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				m_StaticModels[i]->Draw(_backend, _CurrentFrame, i);
				++count;
			}
#pragma endregion
#pragma region DEBUG
			//Debug
			DebugUniformBufferObject dubo{};
			dubo.view = viewMat;
			dubo.projection = projMat;
			memcpy(_backend->m_DbgUniformBuffersMapped[_CurrentFrame], &dubo, sizeof(dubo));
			
			int debugCount = 0;
#if 0
			for (auto& model : modelsToDraw)
			{
				DynamicBufferObject dynO{};
				dynO.model = model->m_ModelMatrix;
				//dynO.model = glm::translate(dynO.model, g_DirectionalLight->m_Pos);
				//dynO.model = glm::scale(dynO.model, glm::vec3(1.f) * g_debugScale);
				//dynO.model = glm::rotate<float>(dynO.model, g_Rotation, m_Rotation);

				uint32_t dynamicOffset = debugCount * static_cast<uint32_t>(dynamicAlignment);
				VkBuffer vertesBuffers[] = { model->m_VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				memcpy((char*)_backend->m_DbgDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_PipelineLayout, 0, 1, 
					&model->m_Material->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
				vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
				vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(model->m_Vertices.size()), 1, 0, 0);

				// Flush to make changes visible to the host
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = _backend->m_DbgDynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
				++debugCount;
			}
#endif
		
#pragma endregion
#pragma region CUBEMAP
			if(g_DrawCubemap)
				DrawCubemapScene(_backend, _CurrentFrame, projMat, viewMat, static_cast<uint32_t>(dynamicAlignment));
#pragma endregion
			// TODO Draw quads.r

			g_editor->Draw(_backend->m_CommandBuffer[_CurrentFrame]);
			_backend->EndRenderPass(_CurrentFrame);
			_backend->SubmitAndPresent(_CurrentFrame, &imageIdx);
			if(g_GPUTimestamp)
				_backend->CollectGPUTimestamps(_CurrentFrame);
		}

		void Scene::PrepareCubemapScene(VKBackend* _backend)
		{
			PERF_INIT("PREPARE_CUBEMAP")
			auto renderContext = GetVKContext();
			/// N - Actualizar los DynamicDescriptorBuffers
			m_Cubemap->m_Material->PrepareMaterialToDraw(_backend);
			/// 5 - Crear buffers de vertices
			void* data;
			VkDeviceSize bufferSize = sizeof(m_Cubemap->m_Vertices[0]) * m_Cubemap->m_Vertices.size();
			// Stagin buffer
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
			vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Cubemap->m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_Cubemap->m_VertexBuffer, m_Cubemap->m_VertexBufferMemory);
			CopyBuffer(m_Cubemap->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool, renderContext.m_GraphicsQueue);
			vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
			vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

			m_Cubemap->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_CubemapUniformBuffers, _backend->m_CubemapDynamicBuffers);
			PERF_END("PREPARE_CUBEMAP")
		}

		void Scene::DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment)
		{
			// Cubemap draw
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_Pipeline);
			auto renderContext = GetVKContext();
			constexpr int sizeCUBO = sizeof(CubemapUniformBufferObject);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(m_CameraFOV), static_cast<float>(g_WindowWidth / g_WindowHeight), zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			glm::mat4 viewMat = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			CubemapUniformBufferObject cubo {};
			cubo.projection = projMat;
			cubo.view = viewMat;
			memcpy(_backend->m_CubemapUniformBuffersMapped[_CurrentFrame], &cubo, sizeof(cubo));
			constexpr int sizeDynO = sizeof(DynamicBufferObject);
			DynamicBufferObject dynO{};
			dynO.model = glm::mat4(1.f);
			dynO.model = glm::scale(dynO.model, glm::vec3(1.f) * g_cubemapDistance);
			uint32_t dynamicOffset = 0 * static_cast<uint32_t>(_dynamicAlignment);
			// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
			memcpy((char*)_backend->m_CubemapDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
			// Update Uniform buffers

			VkBuffer vertesBuffers[] = { m_Cubemap->m_VertexBuffer };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_PipelineLayout, 0, 1,
				&m_Cubemap->m_Material->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
			vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
			vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(m_Cubemap->m_Vertices.size()), 1, 0, 0);
			// Flush to make changes visible to the host
			VkMappedMemoryRange mappedMemoryRange{};
			mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedMemoryRange.memory = _backend->m_CubemapDynamicBuffersMemory[_CurrentFrame];
			mappedMemoryRange.size = sizeof(dynO);
			vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
		}

		void Scene::PrepareScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			/// 1 - Actualizar los DynamicDescriptorBuffers
			//_backend->GenerateBuffers();
			/*VkDeviceSize dynBufferSize = m_StaticModels.size() * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_backend->m_DynamicBuffers[i], _backend->m_DynamicBuffersMemory[i]);
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_DynamicBuffersMemory[i], 0,
					dynBufferSize, 0, &_backend->m_DynamicBuffersMapped[i]);
			}*/
			PERF_INIT("PREPARE_DRAW_SCENE")
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				for (auto& mesh : model->m_Meshes)
				{
					PERF_INIT("PREPARE_MESH")
					/// 5 - Crear buffers de vertices
					#pragma region BUFFER_VERTICES
					{
						void* data;
						if (mesh->m_Vertices.size() <= 0)
						{
							fprintf(stderr, "There is no Triangles to inser on the buffer");
							exit(-57);
						}
						VkDeviceSize bufferSize = sizeof(mesh->m_Vertices[0]) * mesh->m_Vertices.size();
						// Stagin buffer
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
						vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
						memcpy(data, mesh->m_Vertices.data(), (size_t)bufferSize);
						vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
						for (int f = 0; f < FRAMES_IN_FLIGHT; f++)
						{
							CreateBuffer(bufferSize,
								VK_BUFFER_USAGE_TRANSFER_DST_BIT |
								VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
								VK_SHARING_MODE_CONCURRENT,
								VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
								mesh->m_VertexBuffer[f], mesh->m_VertexBufferMemory[f]);
							CopyBuffer(mesh->m_VertexBuffer[f], _backend->m_StagingBuffer, bufferSize
								, _backend->m_CommandPool, renderContext.m_GraphicsQueue);
						}
					}
					#pragma endregion
					// vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
					// vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);
					#pragma region INDEX_BUFFER
					{
						VkDeviceSize bufferSize;
						void* data;
						if (mesh->m_Indices.size() > 0)
						{
							bufferSize = sizeof(mesh->m_Indices[0]) * mesh->m_Indices.size();
							/// 6 - Crear Buffers de Indices
							CreateBuffer(bufferSize,
								VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
								VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
								VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
								_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
							vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
							memcpy(data, mesh->m_Indices.data(), (size_t)bufferSize);
							vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
							for (int f = 0; f < FRAMES_IN_FLIGHT; f++)
							{
								// Index buffer
								CreateBuffer(bufferSize,
									VK_BUFFER_USAGE_TRANSFER_DST_BIT |
									VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									VK_SHARING_MODE_CONCURRENT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									mesh->m_IndexBuffer[f], mesh->m_IndexBufferMemory[f]);
								CopyBuffer(mesh->m_IndexBuffer[f], _backend->m_StagingBuffer, bufferSize
									, _backend->m_CommandPool, renderContext.m_GraphicsQueue);
							}
							// vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
							// vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);
						}
					}
					#pragma endregion
					#pragma region GPU_PARTICLE
					{
						VkDeviceSize bufferSize = sizeof(GPU::Particle);
						GPU::Particle p;
						void* dataParticle;
						// Init info for Compute buffer
						vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &dataParticle);
						memcpy(dataParticle, &p, (size_t)bufferSize);
						vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
						for (int f = 0; f < FRAMES_IN_FLIGHT; f++)
						{
							// Compute Shader buffer
							CreateBuffer(bufferSize,
									VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
									VK_SHARING_MODE_CONCURRENT,
									VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									mesh->m_ComputeBuffer[f], mesh->m_ComputeBufferMemory[f]);
							CopyBuffer(mesh->m_ComputeBuffer[f], _backend->m_StagingBuffer, bufferSize
								, _backend->m_CommandPool, renderContext.m_GraphicsQueue);
						}
					}
					#pragma endregion
					PERF_END("PREPARE_MESH")
					PERF_INIT("PREPARE_MATERIAL")
					model->m_Materials[mesh->m_Material]->PrepareMaterialToDraw(_backend);
					PERF_END("PREPARE_MATERIAL")
					/// 8 - Actualizar Descrip Sets(UpdateDescriptorSet)
					/*model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageView = _backend->m_ShadowImageView;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImage = _backend->m_ShadowImage;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageMem = _backend->m_ShadowImageMemory;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->m_Sampler = _backend->m_ShadowImgSamp;*/
					PERF_INIT("UPDATE_DESCRIPTORS")
					model->m_Materials[mesh->m_Material]->UpdateDescriptorSet(renderContext.m_LogicDevice,
						_backend->m_UniformBuffers, _backend->m_DynamicBuffers, _backend->m_LightsBuffers, _backend->m_ComputeUniformBuffers);
					PERF_END("UPDATE_DESCRIPTORS")
				}
			}
			PERF_END("PREPARE_DRAW_SCENE")
			/// 8 - (OPCIONAL)Reordenar modelos
			// Vamos a pre-ordenar los modelos para pintarlos segun el material.
			// BUBBLESORT de primeras, luego ya veremos, al ser tiempo pre-frameloop, no deberia importar.
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				for (int i = 0; i < model->m_Meshes.size(); i++)
				{
					for (int j = 1; j < model->m_Meshes.size(); j++)
					{
						auto& mesh = model->m_Meshes[i];
						if (model->m_Meshes[j]->m_Material > model->m_Meshes[i]->m_Material)
						{
							auto tempMesh = model->m_Meshes[j];
							model->m_Meshes[j] = model->m_Meshes[i];
							model->m_Meshes[i] = tempMesh;
						}
					}
				}
			}
		}

		void Scene::Init(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			m_Cubemap = new R_Cubemap("resources/Textures/cubemaps/cubemaps_skybox_3.png");
			m_StaticModels[0] = new R_Model("resources/models/Sponza/glTF/", "Sponza.gltf");
			m_CurrentStaticModels = 1;
			g_DirectionalLight = new Directional();
			//g_DirectionalLight->m_LightVisual->m_Materials[0]->CreateDescriptor(renderContext.m_LogicDevice);
			g_PointLights[0] = new Point();
			g_PointLights[1] = new Point();
			g_PointLights[2] = new Point();
			g_PointLights[3] = new Point();
			//PrepareDebugScene(_backend);
			//g_DirectionalLight->m_LightVisual->m_Materials[0]->UpdateDescriptorSet(renderContext.m_LogicDevice,
				//_backend->m_UniformBuffers, _backend->m_DynamicBuffers, _backend->m_LightsBuffers);
			PrepareCubemapScene(_backend);
			g_editor = new Editor(VKR::render::m_Window, _backend->m_Instance, _backend->m_Capabilities.minImageCount,
		_backend->m_SwapchainImagesCount);
			m_SceneDirty = true;
		}

		void Scene::Update()
		{
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				m_StaticModels[i]->Update();
			}
			//g_DirectionalLight->m_LightVisual->Update();
		}
#if 0
		void Scene::PrepareDebugScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
            _backend->m_CurrentDebugModelsToDraw = static_cast<uint32_t>(m_DbgModels.size());
			if(m_DbgModels.size() > 0)
				_backend->GenerateDBGBuffers();
			for (auto& dbgModel : m_DbgModels)
			{
				//dbgModel->m_Material->m_Texture->LoadTexture();
				/// 2 - Crear descriptor pool de materiales(CreateDescPool)`
				dbgModel->m_Material->CreateDescriptor(renderContext.m_LogicDevice);

				/// 3 - Crear Descriptor set de material(createMeshDescSet)
				//dbgModel->m_Material->CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_DbgRender->m_DescSetLayout);
				/// 4 - Crear y transicionar texturas(CreateAndTransImage)
				//dbgModel->m_Material->m_Texture->CreateAndTransitionImageNoMipMaps(_backend->m_CommandPool);
				/// 5 - Crear buffers de vertices
				void* data;
				VkDeviceSize bufferSize = sizeof(dbgModel->m_Vertices[0]) * dbgModel->m_Vertices.size();
				// Stagin buffer
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
				memcpy(data, dbgModel->m_Vertices.data(), (size_t)bufferSize);
				vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					dbgModel->m_VertexBuffer, dbgModel->m_VertexBufferMemory);
				CopyBuffer(dbgModel->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
				vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
				vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

				//dbgModel->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_DbgUniformBuffers, _backend->m_DbgDynamicBuffers);

			}
			
			//g_DirectionalLight->m_LightVisual->m_Material->m_Texture->LoadTexture();
			/// 2 - Crear descriptor pool de materiales(CreateDescPool)`
			/// 3 - Crear Descriptor set de material(createMeshDescSet)
			g_DirectionalLight->m_LightVisual->m_Material->CreateDescriptor(renderContext.m_LogicDevice);

			/// 4 - Crear y transicionar texturas(CreateAndTransImage)
			//g_DirectionalLight->m_LightVisual->m_Material->m_Texture->CreateAndTransitionImageNoMipMaps(_backend->m_CommandPool);
			/// 5 - Crear buffers de vertices
			void* data;
			VkDeviceSize bufferSize = sizeof(g_DirectionalLight->m_LightVisual->m_Vertices[0]) * g_DirectionalLight->m_LightVisual->m_Vertices.size();
			// Stagin buffer
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
			vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, g_DirectionalLight->m_LightVisual->m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				g_DirectionalLight->m_LightVisual->m_VertexBuffer, g_DirectionalLight->m_LightVisual->m_VertexBufferMemory);
			CopyBuffer(g_DirectionalLight->m_LightVisual->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
			vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
			vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

			g_DirectionalLight->m_LightVisual->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_DbgUniformBuffers, _backend->m_DbgDynamicBuffers, _backend->m_LightsBuffers);
		}
#endif
		void Scene::Cleanup(VkDevice _LogicDevice)
		{
			printf("Scene Cleanup\n");
			vkDeviceWaitIdle(_LogicDevice);
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				for (auto& [idx, mat] : model->m_Materials)
				{
					mat->Cleanup(_LogicDevice);
				}
				for (auto& mesh : model->m_Meshes)
				{
					mesh->Cleanup(_LogicDevice);
				}
			}
#if 0
			for (auto& model : m_DbgModels)
			{
				model->Cleanup(_LogicDevice);
			}
#endif
			g_DirectionalLight->Cleanup(_LogicDevice);
			m_Cubemap->Cleanup(_LogicDevice);
			g_editor->Cleanup();
			g_editor->Shutdown();
		}
	}
}
