#include "VKRModel.h"
#include "DebugModels.h"
#include "../../filesystem/ResourceManager.h"
#include "../../perfmon/Custom.h"
#include "../../video/GPUParticle.h"
#include "../../video/VKBackend.h"
#include "../../Camera.h"
#include <cstddef>
#include <cstring>
#include <signal.h>
#include "../../video/VKRUtils.h"


namespace VKR
{
	namespace render
	{
		R_Model::R_Model()
		{
		}

		R_Model::~R_Model()
		{

		}

		R_Model::R_Model(const char* _modelName)
		{
			memset(m_Path, (int)'F', 64);
			RM::_AddRequest(VKR::RM::LOAD, ASSIMP_MODEL, MODELS_PATH, _modelName, this);
		}

		void R_Model::Draw(VKBackend* _backend, int _CurrentFrame, int _countModel)
		{
			// Update Uniform buffers
			UniformBufferObject ubo{};
			ubo.view = g_ViewMatrix;
			ubo.projection = g_ProjectionMatrix;
			ubo.cameraPosition = camera.g_CameraPos;
			memcpy(m_UniformsBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));
			auto renderContext = utils::GetVKContext();
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			auto lightDynAlign = sizeof(LightBufferObject);
			lightDynAlign = (lightDynAlign + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);

			uint32_t lightDynamicOffset0 = (0) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset1 = (1) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset2 = (2) * lightDynamicOffset1;
			uint32_t lightDynamicOffset3 = (3) * lightDynamicOffset1;

			glm::mat4 lightViewMat = glm::lookAt(g_DirectionalLight->m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
			//glm::mat4 lightProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
			glm::mat4 lightProjMat = glm::ortho(-g_DirectionalLight->m_Right, g_DirectionalLight->m_Right, -g_DirectionalLight->m_Up, g_DirectionalLight->m_Up, zNear, zFar);
			std::vector<LightBufferObject> m_LightsOs;
			LightBufferObject temp;
			temp = {};
			temp.addOpts = glm::vec4(g_DirectionalLight->m_Pos, 1.0);
			temp.Opts.x = g_ShadowBias;
			temp.Opts.y = g_ShadowBias;
			temp.View = lightViewMat;
			temp.Proj = lightProjMat;
			temp.Position = glm::vec4(g_DirectionalLight->m_Pos, 1.0);
			temp.Color = glm::vec4(g_DirectionalLight->m_Color, 1.0);
			m_LightsOs.push_back(temp);
			//Point Ligts
			for (int i = 0; i < 3; i++)
			{
				Point& light = g_PointLights[i];
				glm::mat4 lightViewMat = glm::lookAt(light.m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
				glm::mat4 lightProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
				LightBufferObject temp;
				temp = {};
				temp.addOpts = glm::vec4(light.m_Pos, 1.0);
				temp.Opts.x = g_ShadowBias;
				temp.Opts.y = g_ShadowBias;
				temp.addOpts.x = light.m_Kc;
				temp.addOpts.y = light.m_Kl;
				temp.addOpts.z = light.m_Kq;
				temp.View = lightViewMat;
				temp.Proj = lightProjMat;
				temp.Position = glm::vec4(light.m_Pos, 1.0);
				temp.Color = glm::vec4(light.m_Color, 1.0);
				m_LightsOs.push_back(temp);
			}
			memcpy((char*)_backend->m_LightsBuffersMapped[_CurrentFrame] + lightDynamicOffset0
					, m_LightsOs.data(), m_LightsOs.size() * sizeof(LightBufferObject));
			if (g_GPUTimestamp)
				vkCmdWriteTimestamp(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _backend->m_PerformanceQuery[_CurrentFrame], 0);
			int n_mesh = 0;
			for (auto& mesh : m_Meshes)
			{
				uint32_t dynamicOffset = n_mesh * static_cast<uint32_t>(dynamicAlignment);
				R_Material* material = m_Materials[mesh->m_Material];
				DynamicBufferObject dynO{};
				// Update Uniform buffers
				dynO.model = mesh->m_ModelMatrix;
				dynO.modelOpts.x = m_ProjectShadow; // project shadow
				dynO.modelOpts.y = g_MipLevel;
				dynO.addOpts.x = 1;
				dynO.addOpts.y = static_cast<float>(g_ToneMapping);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)m_DynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				VkDeviceSize offsets[] = { 0 };
				std::vector<uint32_t> dynOffsets = {
					dynamicOffset,
					lightDynamicOffset0,
					lightDynamicOffset1,
					lightDynamicOffset2,
					lightDynamicOffset3				};
				VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer[_CurrentFrame] };
				vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, material->material->pipeline.pipeline);
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS
					, material->material->pipeline.layout, 0, 1
					, &m_Materials[mesh->m_Material]->material->materialSets[_CurrentFrame]
					, dynOffsets.size(), dynOffsets.data());
				vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
				// Draw Loop
				if (mesh->m_Indices.size() > 0)
				{
					vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer[_CurrentFrame], 0, VK_INDEX_TYPE_UINT16);
					vkCmdDrawIndexed(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
				}
				// Flush to make changes visible to the host
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = m_DynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
				VkMappedMemoryRange lightsMappedMemoryRange{};
				lightsMappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				lightsMappedMemoryRange.memory = _backend->m_LightsBuffersMemory[_CurrentFrame];
				lightsMappedMemoryRange.size = m_LightsOs.size() * sizeof(LightBufferObject);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &lightsMappedMemoryRange);

#if 0
				ComputeBufferObject cubo;
				cubo.color = glm::vec4(1.0);
				cubo.pos_vel = glm::vec4(0,0,0,1);
				memcpy((char*)_backend->m_ComputeUniformBuffersMapped[_CurrentFrame]
						, &cubo, 1 * sizeof(ComputeBufferObject));
				VkCommandBuffer compute_command_buffer = BeginSingleTimeCommandBuffer(_backend->m_CommandPool);
				vkCmdBindPipeline(compute_command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, material->material->pipeline.compute);
				vkCmdBindDescriptorSets(compute_command_buffer
					, VK_PIPELINE_BIND_POINT_COMPUTE
					, m_Materials[mesh->m_Material]->material->pipeline.compute_layout
					, 0, 1
					, &m_Materials[mesh->m_Material]->material->compute_descriptors_sets[_CurrentFrame]
					, 0, nullptr);
				//vkCmdDispatch(compute_command_buffer, 16, 16, 1);
				EndSingleTimeCommandBuffer(compute_command_buffer, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
#endif
				n_mesh++;
			}
			if (g_GPUTimestamp)
				vkCmdWriteTimestamp(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _backend->m_PerformanceQuery[_CurrentFrame], 1);
		}

		void R_Model::Prepare(VKBackend* _backend)
		{
			auto renderContext = utils::GetVKContext();
			PERF_INIT("PREPARE_MESH")
			// Uniform buffers
			m_UniformBuffers.resize(FRAMES_IN_FLIGHT);
			m_UniformBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_UniformsBuffersMapped.resize(FRAMES_IN_FLIGHT);
			// Dynamic buffers
			m_DynamicBuffers.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMemory.resize(FRAMES_IN_FLIGHT);
			m_DynamicBuffersMapped.resize(FRAMES_IN_FLIGHT);
			GenerateDynamicAndUniformBuffers();
			int count = 0;
			for (auto& mesh : m_Meshes)
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
				utils::CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
				memcpy(data, mesh->m_Vertices.data(), (size_t)bufferSize);
				vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
				for (int f = 0; f < FRAMES_IN_FLIGHT; f++)
				{
					utils::CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mesh->m_VertexBuffer[f], mesh->m_VertexBufferMemory[f]);
					utils::CopyBuffer(mesh->m_VertexBuffer[f], _backend->m_StagingBuffer, bufferSize
						, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
				}

#pragma endregion
				// vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
				// vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);
#pragma region INDEX_BUFFER
				if (mesh->m_Indices.size() > 0)
				{
					bufferSize = sizeof(mesh->m_Indices[0]) * mesh->m_Indices.size();
					/// 6 - Crear Buffers de Indices
					utils::CreateBuffer(bufferSize,
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
						utils::CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							mesh->m_IndexBuffer[f], mesh->m_IndexBufferMemory[f]);
						utils::CopyBuffer(mesh->m_IndexBuffer[f], _backend->m_StagingBuffer, bufferSize
							, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
					}
					// vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
					// vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);
				}
#pragma endregion
#pragma region GPU_PARTICLE
				bufferSize = sizeof(GPU::Particle);
				GPU::Particle p;
				void* dataParticle;
				// Init info for Compute buffer
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &dataParticle);
				memcpy(dataParticle, &p, (size_t)bufferSize);
				vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
				for (int f = 0; f < FRAMES_IN_FLIGHT; f++)
				{
					// Compute Shader buffer
					utils::CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mesh->m_ComputeBuffer[f], mesh->m_ComputeBufferMemory[f]);
					utils::CopyBuffer(mesh->m_ComputeBuffer[f], _backend->m_StagingBuffer, bufferSize
						, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
				}
#pragma endregion
				PERF_END("PREPARE_MESH")
				PERF_END("PREPARE_MESH")
				PERF_INIT("PREPARE_MATERIAL")
				m_Materials[mesh->m_Material]->PrepareMaterialToDraw(_backend);
				PERF_END("PREPARE_MATERIAL")
				PERF_INIT("UPDATE_DESCRIPTORS")
				m_Materials[mesh->m_Material]->UpdateDescriptorSet(renderContext.m_LogicDevice,
						m_UniformBuffers, m_DynamicBuffers, _backend->m_LightsBuffers, m_ComputeUniformBuffers);
				PERF_END("UPDATE_DESCRIPTORS")

				++count;
			}
		}
		void R_Model::GenerateDynamicAndUniformBuffers()
		{
#pragma region RENDER_BUFFERS
			VkDeviceSize bufferSize = sizeof(UniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_UniformBuffers[i], m_UniformBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_UniformBuffersMemory[i], 0,
					bufferSize, 0, &m_UniformsBuffersMapped[i]);
			}
			auto DynAlign = sizeof(DynamicBufferObject);
			DynAlign = (DynAlign + utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(utils::g_context.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			VkDeviceSize checkBufferSize = m_Meshes.size() * DynAlign;
			VkDeviceSize dynBufferSize = m_Meshes.size() * sizeof(DynamicBufferObject);
			if (dynBufferSize != checkBufferSize)
#ifdef _MSVC
				__debugbreak();
#else
					raise(SIGTRAP);
#endif
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				utils::CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DynamicBuffers[i], m_DynamicBuffersMemory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, m_DynamicBuffersMemory[i], 0,
					dynBufferSize, 0, &m_DynamicBuffersMapped[i]);
			}
#pragma endregion
		}
		void R_Model::Cleanup()
		{
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_UniformBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_UniformBuffersMemory[i], nullptr);

				vkDestroyBuffer(utils::g_context.m_LogicDevice, m_DynamicBuffers[i], nullptr);
				vkFreeMemory(utils::g_context.m_LogicDevice, m_DynamicBuffersMemory[i], nullptr);
			}
		}
	}
}

