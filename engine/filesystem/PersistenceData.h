#ifndef _C_PERSISTENCE_DATA
#define _C_PERSISTENCE_DATA
#include "Resource.h"
#include "../video/VKBackend.h"
#include "../core/Objects/VKRModel.h"
#include <vector>
#include <mutex>
namespace VKR
{
	void SaveCurrentState(render::VKBackend* _backend)
	{
		/*while (!_backend->BackendShouldClose())
		{
			int b = 10;
		}*/
	}
	
	static std::vector<sResource*> toLoadResourcesAsync;
	static std::vector<sResource*> loadedResourcesAsync;
	std::mutex mx_content;
	render::R_Model* tempModel;

	void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture = nullptr)
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
			if (tempModel->m_Materials[mesh->mMaterialIndex] == nullptr &&
				_customTexture == nullptr && _scene->HasMaterials())
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
			// else if (_customTexture != nullptr) // renemos que crear el modelo con textura custom
			// {
			// 	tempModel->m_Materials[mesh->mMaterialIndex] = new R_Material();
			// 	tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureDiffuse = new Texture(std::string(_customTexture));
			// 	tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureSpecular = new Texture(std::string(_customTexture));
			// 	tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureAmbient = new Texture(std::string(_customTexture));
			// 	tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureShadowMap = new Texture();
			// }
			tempMesh->m_Material = mesh->mMaterialIndex;
			//++m_TotalTextures;
			tempModel->m_Meshes.push_back(tempMesh);
		}
	}

	void LoadAsyncResource(const char* _path)
	{
		printf("\nLoading %s\n", _path);
		const aiScene* scene = aiImportFile(_path, aiProcess_Triangulate);
		if (!scene->HasMeshes())
			exit(-225);
		tempModel = new render::R_Model();
		//Process Node
		auto node = scene->mRootNode;
		ProcessModelNode(node, scene, _path);
		// Insert new static model
		sprintf(tempModel->m_Path,"%s", _path);
		// tempModel->m_Pos = glm::vec3(0.f);
		// tempModel->m_Scale = _scale;
	}

	void AddAsyncRequest(sResource* _request)
	{
		mx_content.lock();
		toLoadResourcesAsync.push_back(new sResource(_request->m_PathFromLoad));
		mx_content.unlock();
	}

	void ContentManager(render::VKBackend* _backend)
	{
		while(!_backend->BackendShouldClose())
		{
			int a = 10;
			if(toLoadResourcesAsync.size() > 0)
			{
				mx_content.lock();
				printf("Loading Resource");
				for (sResource* resource : toLoadResourcesAsync)
				{
					char filename[256];
					sprintf(filename, "%s",resource->m_PathFromLoad);
					LoadAsyncResource(filename);
					resource->m_State=READY;
					render::m_StaticModels.push_back(tempModel);
					render::m_SceneDirty = true;
				}
				toLoadResourcesAsync.pop_back();
				mx_content.unlock();
			}
		}
	}
}
#endif