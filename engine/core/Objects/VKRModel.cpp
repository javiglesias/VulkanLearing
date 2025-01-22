#include "VKRModel.h"
#include "DebugModels.h"
#include "../../filesystem/ResourceManager.h"
#include "../../perfmon/Custom.h"
#include <cstddef>
#include <cstring>


namespace VKR
{
	namespace render
	{
		R_Model::R_Model()
		{
			memset(m_Path, (int)'F', 64);
		}

		R_Model::~R_Model()
		{

		}

		R_Model::R_Model(const char* _filepath, const char* _modelName)
		{
			strcpy(m_Path, _filepath);
			RM::_AddRequest(TYPE::STATIC_MODEL, _filepath,_modelName, this);
		}

		void R_Model::Draw(VKBackend* _backend, int _CurrentFrame, int _countModel)
		{
			auto renderContext = GetVKContext();
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			auto lightDynAlign = sizeof(LightBufferObject);
			lightDynAlign = (lightDynAlign + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);

			uint32_t dynamicOffset = _countModel * static_cast<uint32_t>(dynamicAlignment);
			uint32_t lightDynamicOffset0 = (0) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset1 = (1) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset2 = (2) * lightDynamicOffset1;
			uint32_t lightDynamicOffset3 = (3) * lightDynamicOffset1;

			glm::mat4 lightViewMat = glm::lookAt(g_DirectionalLight->m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
			glm::mat4 lightProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
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
				Point* light = g_PointLights[i];
				glm::mat4 lightViewMat = glm::lookAt(light->m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
				glm::mat4 lightProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
				LightBufferObject temp;
				temp = {};
				temp.addOpts = glm::vec4(light->m_Pos, 1.0);
				temp.Opts.x = g_ShadowBias;
				temp.Opts.y = g_ShadowBias;
				temp.addOpts.x = light->m_Kc;
				temp.addOpts.y = light->m_Kl;
				temp.addOpts.z = light->m_Kq;
				temp.View = lightViewMat;
				temp.Proj = lightProjMat;
				temp.Position = glm::vec4(light->m_Pos, 1.0);
				temp.Color = glm::vec4(light->m_Color, 1.0);
				m_LightsOs.push_back(temp);
			}
			memcpy((char*)_backend->m_LightsBuffersMapped[_CurrentFrame] + lightDynamicOffset0, m_LightsOs.data(), m_LightsOs.size() * sizeof(LightBufferObject));
			if (m_Hidden) return;
			if (g_GPUTimestamp)
				vkCmdWriteTimestamp(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _backend->m_PerformanceQuery[_CurrentFrame], 0);
			for (auto& mesh : m_Meshes)
			{
				R_Material* material = m_Materials[mesh->m_Material];
				DynamicBufferObject dynO{};
				// Update Uniform buffers
				dynO.model = mesh->m_ModelMatrix;
				dynO.modelOpts.x = m_ProjectShadow; // project shadow
				dynO.modelOpts.y = g_MipLevel;
				dynO.addOpts.x = 1;
				dynO.addOpts.y = static_cast<float>(g_ToneMapping);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_DynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				VkDeviceSize offsets[] = { 0 };
				std::vector<uint32_t> dynOffsets = {
					dynamicOffset,
					lightDynamicOffset0,
					lightDynamicOffset1,
					lightDynamicOffset2,
					lightDynamicOffset3				};
				VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer[_CurrentFrame] };
				vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, material->material->pipeline.pipeline);
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, material->material->pipeline.layout, 0, 1,
					&m_Materials[mesh->m_Material]->material->materialSets[_CurrentFrame], dynOffsets.size(), dynOffsets.data());
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
				mappedMemoryRange.memory = _backend->m_DynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
				VkMappedMemoryRange lightsMappedMemoryRange{};
				lightsMappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				lightsMappedMemoryRange.memory = _backend->m_LightsBuffersMemory[_CurrentFrame];
				lightsMappedMemoryRange.size = sizeof(LightBufferObject);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &lightsMappedMemoryRange);
				vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, material->material->pipeline.compute);
			}
			if (g_GPUTimestamp)
				vkCmdWriteTimestamp(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _backend->m_PerformanceQuery[_CurrentFrame], 1);
		}

		void R_Model::Update()
		{
			for (auto& mesh : m_Meshes)
			{
				mesh->m_ModelMatrix = m_ModelMatrix;
			}
		}
	}
}

