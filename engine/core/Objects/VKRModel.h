#ifndef _C_MODEL
#define _C_MODEL
#include "../Materials/VKRMaterial.h"
#include "../Materials/VKRDebugMaterial.h"
#include "../../video/Types.h"

#include <unordered_map>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>


namespace VKR
{
	namespace render
	{
		enum PRIMITIVE : uint8_t
		{
			SPHERE,
			ARROW,
			QUAD,
			TERRAIN,
			CUBE
		};
		struct R_Mesh
		{// lo necesario para poder renderizar una Malla
		public:
			std::vector<Vertex3D> m_Vertices;
			std::vector<uint16_t> m_Indices;
			// Base_color, metallicRoughtness, normal Textures
			uint32_t m_Material;
			VkBuffer m_VertexBuffer = nullptr;
			VkDeviceMemory m_VertexBufferMemory = nullptr;
			VkBuffer m_IndexBuffer = nullptr;
			VkDeviceMemory m_IndexBufferMemory = nullptr;
		public:
			R_Mesh() {}
			void Cleanup(VkDevice _LogicDevice);
		};
		
		struct R_Model //Render Model
		{// lo necesario para poder renderizar un Modelo
		public:
			R_Model();
		public:
			glm::mat4 m_ModelMatrix = glm::mat4(1.f);
			glm::vec3 m_Pos { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_RotAngle { 0.0f, 1.0f, 0.0f };
			glm::vec3 m_Scale { 1.0f, 1.0f, 1.0f };
			std::vector<R_Mesh*> m_Meshes;
			std::unordered_map<uint32_t, R_Material*> m_Materials;
			char m_Path[64];
			bool m_Editable = false;
			float m_RotGRAD = 0.f;
			float m_ProjectShadow = 0.f;
		};

		struct R_DbgModel
		{
		public:
			R_DbgMaterial* m_Material;
			VkBuffer m_VertexBuffer;
			VkDeviceMemory m_VertexBufferMemory;
			glm::vec3 m_Pos{ 0.0f };
			glm::vec3 m_Rotation{ 0.0f };
			glm::mat4 m_ModelMatrix = glm::mat4{ 1.0 };
			std::vector<DBG_Vertex3D> m_Vertices;
		public:
			R_DbgModel(const char* _filepath, const char* _modelName);
			void Cleanup(VkDevice _LogicDevice);
		};
	}
}
#endif