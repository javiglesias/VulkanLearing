#include "ResourceManager.h"
#include "gltfReader.h"
#include "../core/Objects/VKRModel.h"
#include "../video/VKBackend.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace VKR
{
	namespace RM
	{
		render::R_Model* tempModel;
		bool LoadModel_ALT(const char* _filepath, const char* _modelName)
		{
			char filename[128];
			tempModel = new render::R_Model();
			sprintf(filename, "%s%s", _filepath, _modelName);
			sprintf(tempModel->m_Path , "%s",  _modelName);
			auto data = filesystem::read_glTF(_filepath, _modelName, tempModel);
			if(data == nullptr) return false;
			render::m_PendingBuffersModels[render::m_CurrentPendingModels] = tempModel;
			render::m_CurrentPendingModels++;
			return true;
		}

		void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath)
		{
			// CHILDREN
			for (unsigned int i = 0; i < _node->mNumChildren; i++)
			{
				ProcessModelNode(_node->mChildren[i], _scene, _filepath);
			}
			int lastTexIndex = 0;
			uint32_t tempMaterial = -1;
			for (int m = 0; m < _node->mNumMeshes; m++)
			{
				const aiMesh* mesh = _scene->mMeshes[_node->mMeshes[m]];
				render::R_Mesh* tempMesh = new render::R_Mesh();
				//Process Mesh
				for (unsigned int f = 0; f < mesh->mNumFaces; f++)
				{
					const aiFace& face = mesh->mFaces[f];
					for (unsigned int j = 0; j < face.mNumIndices; j++)
					{
						// m_Indices.push_back(face.mIndices[0]);
						// m_Indices.push_back(face.mIndices[1]);
						// m_Indices.push_back(face.mIndices[2]);
						tempMesh->m_Indices.push_back(face.mIndices[0]);
						tempMesh->m_Indices.push_back(face.mIndices[1]);
						tempMesh->m_Indices.push_back(face.mIndices[2]);
					}
				}
				for (unsigned int v = 0; v < mesh->mNumVertices; v++)
				{
					Vertex3D tempVertex;
					tempVertex.m_Pos = { mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z };
					if (mesh->mTextureCoords[0])
					{
						tempVertex.m_TexCoord = { mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y };
					}
					else
					{
						tempVertex.m_TexCoord = { 0.f, 0.f };
					}
					tempVertex.m_Normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
					// New New Mexico
					tempMesh->m_Vertices.push_back(tempVertex);
				}
				// Textura por Mesh
				int texIndex = 0;
				aiString path;
				if (tempModel->m_Materials[mesh->mMaterialIndex] == nullptr)
				{
					tempModel->m_Materials[mesh->mMaterialIndex] = new render::R_Material();
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
					auto textureDiffuse = std::string(_filepath);
					textureDiffuse += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureDiffuse = new render::Texture(textureDiffuse);
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
					auto textureSpecular = std::string(_filepath);
					textureSpecular += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureSpecular = new render::Texture(textureSpecular);
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT, texIndex, &path);
					auto textureAmbient = std::string(_filepath);
					textureAmbient += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureAmbient = new render::Texture(textureAmbient);
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureShadowMap = new render::Texture();
				}
				tempMesh->m_Material = mesh->mMaterialIndex;
				//++m_TotalTextures;
				tempModel->m_Meshes.push_back(tempMesh);
			}
		}

		void LoadModel(const char* _filepath, const char* _modelName)
		{
			//PERF_INIT()
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
			printf("\nLoading %s\n", _modelName);
			const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
			if (!scene || !scene->HasMeshes())
				exit(-225);
			tempModel = new render::R_Model();
			//Process Node
			auto node = scene->mRootNode;
			ProcessModelNode(node, scene, _filepath);
			// Insert new static model
			sprintf(tempModel->m_Path, _filepath, 64);
			/*tempModel->m_Pos = _position;
			tempModel->m_Scale = _scale;*/
			//PERF_END("LOAD MODEL")
			render::m_PendingBuffersModels[render::m_CurrentPendingModels] = tempModel;
			render::m_CurrentPendingModels++;
		}


		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName)
		{
			_RMRequests[_NumRequests].type = _type;
			strcpy(_RMRequests[_NumRequests].filepath, _filepath);
			strcpy(_RMRequests[_NumRequests].resourceName, _resourceName);
			_NumRequests++;
		}

		void _Init()
		{
			_Loop();
		}

		void _Loop()
		{
			while(true)
			{
				if(_NumRequests > 0)
					switch(_RMRequests[_NumRequests-1].type)
					{
						case STATIC_MODEL:
						{
							LoadModel(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName);
							render::m_SceneDirty = true;
							_NumRequests--;
							break;
						}
						case ASSIMP_MODEL:
						{
							LoadModel(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName);
							render::m_SceneDirty = true;
							_NumRequests--;
							break;
						}
						default:
							break;
					}
			}
			_Destroy();
		}

		void _Destroy()
		{
		}
	}
}
