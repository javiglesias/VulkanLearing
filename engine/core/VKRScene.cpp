#include "VKRScene.h"
#include "../video/VKRUtils.h"
#include "../perfmon/Custom.h"
#include "../editor/Editor.h"
#include "../core/Objects/VKRCubemap.h"

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
			auto renderContext = utils::GetVKContext();
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
			auto renderContext = utils::GetVKContext();
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
			auto renderContext = utils::GetVKContext();
			vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
			vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);
			// GeometryPass(_backend, _CurrentFrame);
			if (g_ShadowPassEnabled)
				ShadowPass(_backend, _CurrentFrame);
			else
				if (m_SceneDirty)
				{
					PrepareScene(_backend);
					m_SceneDirty = false;
				}
			vkCmdResetQueryPool(_backend->m_CommandBuffer[_CurrentFrame], _backend->m_PerformanceQuery[_CurrentFrame], 0, static_cast<uint32_t>(g_Timestamps.size()));
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			auto lightDynAlign = sizeof(LightBufferObject);
			lightDynAlign = (lightDynAlign + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			// Matriz de proyeccion
			g_ProjectionMatrix = glm::perspective(glm::radians(m_CameraFOV), static_cast<float>(g_WindowWidth / g_WindowHeight), zNear, zFar);
			g_ProjectionMatrix[1][1] *= -1; // para invertir el eje Y
			g_ViewMatrix = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			/// Render Pass
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
#pragma region LIGHTS
			for (int l = 0; l < g_Lights.size(); l++)
			{
				g_Lights[l]->Draw(_backend, _CurrentFrame);
			}
			g_DirectionalLight->Draw(_backend, _CurrentFrame);
			/*g_PointLights[0]->Draw(_backend, _CurrentFrame);
			g_PointLights[1]->Draw(_backend, _CurrentFrame);
			g_PointLights[2]->Draw(_backend, _CurrentFrame);
			g_PointLights[3]->Draw(_backend, _CurrentFrame);*/
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
			dubo.view = g_ViewMatrix;
			dubo.projection = g_ProjectionMatrix;
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
			/*if(g_DrawCubemap)
				DrawCubemapScene(_backend, _CurrentFrame, projMat, viewMat, static_cast<uint32_t>(dynamicAlignment));*/
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
			auto renderContext = utils::GetVKContext();
			/// N - Actualizar los DynamicDescriptorBuffers
			m_Cubemap->m_Material->PrepareMaterialToDraw(_backend);
			/// 5 - Crear buffers de vertices
			void* data;
			VkDeviceSize bufferSize = sizeof(m_Cubemap->m_Vertices[0]) * m_Cubemap->m_Vertices.size();
			// Stagin buffer
			utils::CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
			vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Cubemap->m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
			utils::CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_Cubemap->m_VertexBuffer, m_Cubemap->m_VertexBufferMemory);
			utils::CopyBuffer(m_Cubemap->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
			vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
			vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

			m_Cubemap->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_CubemapUniformBuffers, _backend->m_CubemapDynamicBuffers);
			PERF_END("PREPARE_CUBEMAP")
		}

		void Scene::DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment)
		{
			// Cubemap draw
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_Pipeline);
			auto renderContext = utils::GetVKContext();
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
			auto renderContext = utils::GetVKContext();
			PERF_INIT("PREPARE_DRAW_SCENE")
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				m_StaticModels[i]->Prepare(_backend);
			}
			g_DirectionalLight->m_visual_model->Prepare(_backend);
			/*g_PointLights[0]->m_visual_model->Prepare(_backend);
			g_PointLights[1]->m_visual_model->Prepare(_backend);
			g_PointLights[2]->m_visual_model->Prepare(_backend);
			g_PointLights[3]->m_visual_model->Prepare(_backend);*/
			PERF_END("PREPARE_DRAW_SCENE")
		}

		void Scene::Init(VKBackend* _backend)
		{
			auto renderContext = utils::GetVKContext();
			//m_Cubemap = new R_Cubemap("resources/Textures/cubemaps/cubemaps_skybox_3.png");
			m_StaticModels[0] = new R_Model("Sponza");
			m_CurrentStaticModels = 1;
			g_DirectionalLight = new Directional();
			/*g_PointLights[0] = new Point();
			g_PointLights[1] = new Point();
			g_PointLights[2] = new Point();
			g_PointLights[3] = new Point();*/
			//PrepareCubemapScene(_backend);
			g_editor = new Editor(m_Window, _backend->m_Instance, _backend->m_Capabilities.minImageCount,
		_backend->m_SwapchainImagesCount);
			m_SceneDirty = true;
		}

		void Scene::Update()
		{
			/*for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				m_StaticModels[i]->Update();
			}*/
		}
#if 0
		void Scene::PrepareDebugScene(VKBackend* _backend)
		{
			auto renderContext = utils::GetVKContext();
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
			//m_Cubemap->Cleanup(_LogicDevice);
			g_editor->Cleanup();
			g_editor->Shutdown();
		}
	}
}
