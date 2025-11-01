#include "VKRCubemap.h"
#include "DebugModels.h"
#include "../Materials/VKRTexture.h"
#include "../../perfmon/Custom.h"
#include "../../video/VKRUtils.h"
#include "../../Camera.h"

#include <glm/glm.hpp>
#include <cstring>

namespace VKR
{
	namespace render
	{
		R_Cubemap::R_Cubemap(const char* _texturePath)
		{
			fprintf(stdout, "Loading cubemap %s\n", _texturePath);
			strcpy(m_Path, _texturePath);	
			m_Material = new R_CubemapMaterial(_texturePath);
			// TODO creacion de los vertices
			m_Vertices = m_CubeVertices;
		}

		void R_Cubemap::GenerateBuffers()
		{
			constexpr VkDeviceSize bufferSize = sizeof(CubemapUniformBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				utils::CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					uniform_Buffers[i], uniform_Buffers_Memory[i]);
				vkMapMemory(utils::g_context.m_LogicDevice, uniform_Buffers_Memory[i], 0,
					bufferSize, 0, &uniforms_Buffers_Mapped[i]);
			}
		}

		void R_Cubemap::Prepare(VKBackend* _backend)
		{
			PERF_INIT("PREPARE_CUBEMAP");
			auto renderContext = utils::GetVKContext();
			/// N - Actualizar los DynamicDescriptorBuffers
			m_Material->PrepareMaterialToDraw(_backend);
			/// 5 - Crear buffers de vertices
			GenerateBuffers();
			void* data;
			VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
			// Stagin buffer
			utils::CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
			vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
			utils::CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_VertexBuffer, m_VertexBufferMemory);
			utils::CopyBuffer(m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool, renderContext.m_GraphicsComputeQueue);
			vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
			vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

			m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, uniform_Buffers);
			PERF_END("PREPARE_CUBEMAP")
		}
		void R_Cubemap::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
			m_Material->Cleanup(_LogicDevice);
		}
		void R_Cubemap::Draw(VKBackend* _backend, int _CurrentFrame)
		{
#if 1
			// Cubemap draw
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_Pipeline);
			auto renderContext = utils::GetVKContext();
			constexpr int sizeCUBO = sizeof(CubemapUniformBufferObject);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(camera.m_CameraFOV), static_cast<float>(g_WindowWidth / g_WindowHeight), zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			
			CubemapUniformBufferObject cubo{};
			cubo.projection = projMat;
			cubo.model = glm::mat4(1.f);;
			cubo.model = glm::scale(cubo.model, glm::vec3(1.f) * g_cubemapDistance);
			
			memcpy(uniforms_Buffers_Mapped[_CurrentFrame], &cubo, sizeof(cubo));

			VkBuffer vertesBuffers[] = { m_VertexBuffer };
			VkDeviceSize offsets[] = {0};

			vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_PipelineLayout, 0, 1,
				&m_Material->m_DescriptorSet[_CurrentFrame], 0, nullptr);
			vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
			vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
#endif
		}
	}
}
