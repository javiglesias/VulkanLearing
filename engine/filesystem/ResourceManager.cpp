#include "ResourceManager.h"
#include "gltfReader.h"
#include "../core/Materials/VKRTexture.h"
#include "../video/VKBackend.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace VKR
{
	namespace RM
	{
		render::R_Model* tempModel;
		void GenerateTextureMesh(const char* _filepath, aiTextureType _type, unsigned int _texIndex, aiMaterial* _material, unsigned int _matIndex, render::Texture** outTex_);
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
			for (unsigned int m = 0; m < _node->mNumMeshes; m++)
			{
				const aiMesh* mesh = _scene->mMeshes[_node->mMeshes[m]];
				render::VKRenderable* tempMesh = new render::VKRenderable();
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
					if (mesh->mNormals)
						tempVertex.m_Normal = { mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z };
					else
						tempVertex.m_Normal = {0.f, 1.f, 0.f};
					// New New Mexico
					tempMesh->m_Vertices.push_back(tempVertex);
				}
				// Textura por Mesh
				int texIndex = 0;
				aiString path;
				if (tempModel->m_Materials[mesh->mMaterialIndex] == nullptr)
				{
					tempModel->m_Materials[mesh->mMaterialIndex] = new render::R_Material();
					tempModel->m_Materials[mesh->mMaterialIndex]->material.pipeline._buildPipeline();
					for (int t = 0; t < 7; t++)
					{
						GenerateTextureMesh(_filepath, static_cast<aiTextureType>(tex_type[t]), 
							texIndex, _scene->mMaterials[mesh->mMaterialIndex], mesh->mMaterialIndex,
							&tempModel->m_Materials[mesh->mMaterialIndex]->textures[t]);
					}
					tempModel->m_Materials[mesh->mMaterialIndex]->textures[7] = new render::Texture("");
					++tempModel->nMaterials;
				}

				tempMesh->m_Material = mesh->mMaterialIndex;
				//++m_TotalTextures;
				tempModel->m_Meshes.push_back(tempMesh);
			}
		}

		void GenerateTextureMesh(const char* _filepath, aiTextureType _type, unsigned int _texIndex, aiMaterial* _material, unsigned int _matIndex, render::Texture** outTex_)
		{
			aiString path;
			auto diff = _material->GetTexture(_type, _texIndex, &path);
			if (diff == aiReturn_SUCCESS)
			{
				auto texture = std::string(_filepath);
				texture += path.data;
				*outTex_ = new render::Texture(texture);
			}
			else
				*outTex_ = new render::Texture("");
		}

		void LoadModel(const char* _filepath, const char* _modelName, render::R_Model* model_)
		{
			//PERF_INIT()
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
			printf("\nLoading %s\n", filename);
			const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
			if (!scene || !scene->HasMeshes())
				exit(-225);
			if (model_ == nullptr)
				tempModel = new render::R_Model();
			else
				tempModel = model_;
			//Process Node
			auto node = scene->mRootNode;
			ProcessModelNode(node, scene, _filepath);
			sprintf(tempModel->m_Path, "%s%s", _filepath, _modelName);
		}


		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName, render::R_Model* model_)
		{
			if(model_ != nullptr)
				LoadModel(_filepath, _resourceName, model_);
			else
			{
				_RMRequests[_NumRequests].type = _type;
				strcpy(_RMRequests[_NumRequests].filepath, _filepath);
				strcpy(_RMRequests[_NumRequests].resourceName, _resourceName);
				_NumRequests++;
			}
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
							LoadModel_ALT(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName);
							render::m_SceneDirty = true;
							_NumRequests--;
							break;
						}
						case ASSIMP_MODEL:
						{
							LoadModel(_RMRequests[_NumRequests-1].filepath, _RMRequests[_NumRequests-1].resourceName, static_cast<render::R_Model*>(_RMRequests[_NumRequests-1].modelPtr));
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
