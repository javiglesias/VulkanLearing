#include "VKRModel.h"
#include "DebugModels.h"


namespace VKR
{
	namespace render
	{
		void R_Mesh::Cleanup(VkDevice _LogicDevice)
		{
			if (m_Indices.size() > 0)
			{
				vkDestroyBuffer(_LogicDevice, m_IndexBuffer, nullptr);
				vkFreeMemory(_LogicDevice, m_IndexBufferMemory, nullptr);
			}
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
		}

		R_DbgModel::R_DbgModel(PRIMITIVE _type)
		{
			switch(_type)
			{
			case SPHERE:
				AddSphereDebug();
				break;
			case ARROW:
				AddArrowDebug();
				break;
			case CUBE:
				AddCubeDebug();
				break;
			case QUAD:
				AddQuadDebug();
				break;
			case TERRAIN:
				AddTerrainDebug();
				break;
			default:
				break;
			}
		}

		void R_DbgModel::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
			m_Material.Cleanup(_LogicDevice);
		}

		void R_DbgModel::AddSphereDebug()
		{
			m_Vertices = m_SphereVertices;
		}

		void R_DbgModel::AddArrowDebug()
		{
			m_Vertices = m_Arrow;
		}

		void R_DbgModel::AddCubeDebug()
		{
			m_Vertices = m_CubeVertices;
		}

		void R_DbgModel::AddQuadDebug()
		{
			m_Vertices = m_QuadVertices;
		}
		void R_DbgModel::AddTerrainDebug()
		{
			m_Vertices = m_QuadVertices;
			m_Rotation = glm::vec3(0, 1, 1);
		}
		R_Model::R_Model()
		{
			memset(m_Path, (int)'F', 64);
		}
	}
}

