#ifndef _C_MODEL
#define _C_MODEL
#include "../Materials/VKRDebugMaterial.h"
#include "../../video/VKRenderable.h"

#include <unordered_map>

namespace VKR
{
	namespace render
	{
		struct R_Model //Render Model
		{// lo necesario para poder renderizar un Modelo
			R_Model();
			~R_Model();
			void Draw(VKBackend* _backend, int _CurrentFrame, int _countModel = 0);
			void Update();
			R_Model(const char* _filepath, const char* _modelName);
			
			glm::mat4 m_ModelMatrix = glm::mat4(1.f);
			std::vector<VKRenderable*> m_Meshes;
			std::unordered_map<uint32_t, R_Material*> m_Materials;
			char m_Path[64];
			bool m_Editable = false;
			bool m_Hidden = false;
			float m_RotGRAD = 0.f;
			float m_ProjectShadow = 0.f;
		};

		/*struct R_DbgModel
		{
		public:
			R_Material* m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			glm::vec3 m_Pos{ 0.0f };
			glm::vec4 base_color { 0.0f };
			glm::vec3 m_Rotation{ 0.0f };
			glm::mat4 m_ModelMatrix = glm::mat4{ 1.0 };
			std::vector<DBG_Vertex3D> m_Vertices;
			std::vector<uint16_t> m_Indices;
		public:
			R_DbgModel(const char* _filepath, const char* _modelName);
			void Cleanup(VkDevice _LogicDevice);
		};*/
	}
}
#endif