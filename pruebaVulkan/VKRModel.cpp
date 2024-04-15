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
			case PRIMITIVE::SPHERE:
				AddSphereDebug();
				break;
			case CUBE:
				AddCubeDebug();
				break;
			case QUAD:
				AddQuadDebug();
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

		void R_DbgModel::AddCubeDebug()
		{
			m_Vertices = m_CubeVertices;
		}

		void R_DbgModel::AddQuadDebug()
		{
			m_Vertices = m_QuadVertices;
		}
	}
}

