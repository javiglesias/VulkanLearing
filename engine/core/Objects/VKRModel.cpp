#include "VKRModel.h"
#include "DebugModels.h"
#include "../../filesystem/gltfReader.h"
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

		R_DbgModel::R_DbgModel(const char* _filepath, const char* _modelName)
		{
			auto result = filesystem::read_glTF_DBG(_filepath,_modelName, this);
		}

		void R_DbgModel::Cleanup(VkDevice _LogicDevice)
		{
			vkDestroyBuffer(_LogicDevice, m_VertexBuffer, nullptr);
			vkFreeMemory(_LogicDevice, m_VertexBufferMemory, nullptr);
			m_Material->Cleanup(_LogicDevice);
		}

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
	}
}

