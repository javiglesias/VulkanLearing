#include "gltfReader.h"
#include "../../dependencies/cgltf/cgltf.h"
#include <cstddef>
#include "../core/Objects/VKRModel.h"
#include "../core/Materials/VKRTexture.h"
#include "../video/VKRenderable.h"

#include <glm/gtc/matrix_transform.hpp>

#include <string>
#include <signal.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace VKR
{
	namespace filesystem
	{
		render::R_Model* tempModel;

		int tex_type[8] = {
		   aiTextureType_BASE_COLOR,
		   aiTextureType_DIFFUSE_ROUGHNESS,
		   aiTextureType_SPECULAR,
		   aiTextureType_AMBIENT,
		   aiTextureType_NORMALS,
		   aiTextureType_AMBIENT_OCCLUSION,
		   aiTextureType_EMISSION_COLOR,
		   aiTextureType_LIGHTMAP
		};
		bool LoadModel_ALT(const char* _filepath, const char* _modelName)
		{
			char filename[128];
			tempModel = new render::R_Model();
			sprintf(filename, "%s%s", _filepath, _modelName);
			sprintf(tempModel->m_Path, "%s", _modelName);
			auto data = filesystem::read_glTF(_filepath, _modelName, tempModel);
			if (data == nullptr) return false;
			/*render::m_PendingBuffersModels[render::m_CurrentPendingModels] = tempModel;
			render::m_CurrentPendingModels++;*/
			return true;
		}

		void GenerateTextureMesh(const char* _filepath, aiTextureType _type, unsigned int _texIndex, aiMaterial* _material, unsigned int _matIndex, render::Texture** outTex_)
		{
			aiString path;
			auto diff = _material->GetTexture(_type, _texIndex, &path);
			if (diff == aiReturn_SUCCESS)
			{
				char texture[256];
				sprintf(texture, "%s/%s", _filepath, path.data);
				*outTex_ = new render::Texture(texture);
			}
			else
				*outTex_ = new render::Texture("");
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
				strcpy(tempMesh->m_Id, mesh->mName.C_Str());
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
						tempVertex.m_Normal = { 0.f, 0.f, 0.f };
					// New New Mexico
					tempMesh->m_Vertices.push_back(tempVertex);
				}
				// Textura por Mesh
				int texIndex = 0;
				aiString path;
				if (tempModel->m_Materials[mesh->mMaterialIndex] == nullptr)
				{
					tempModel->m_Materials[mesh->mMaterialIndex] = new render::R_Material();
					tempModel->m_Materials[mesh->mMaterialIndex]->material = new render::MaterialInstance();
					tempModel->m_Materials[mesh->mMaterialIndex]->material->pipeline._buildPipeline();

					for (int t = 0; t < MAX_TEXTURES; t++)
					{
						GenerateTextureMesh(_filepath, static_cast<aiTextureType>(tex_type[t]),
							texIndex, _scene->mMaterials[mesh->mMaterialIndex], mesh->mMaterialIndex,
							&tempModel->m_Materials[mesh->mMaterialIndex]->textures[t]);
					}
					++tempModel->nMaterials;
				}

				tempMesh->m_Material = mesh->mMaterialIndex;
				tempMesh->m_ModelMatrix = {
					_node->mTransformation.a1,_node->mTransformation.b1,_node->mTransformation.c1,_node->mTransformation.d1,
					_node->mTransformation.a2,_node->mTransformation.b2,_node->mTransformation.c2,_node->mTransformation.d2,
					_node->mTransformation.a3,_node->mTransformation.b3,_node->mTransformation.c3,_node->mTransformation.d3,
					_node->mTransformation.a4,_node->mTransformation.b4,_node->mTransformation.c4,_node->mTransformation.d4
				};
				tempModel->m_Meshes.push_back(tempMesh);
			}
		}

		void LoadModel(const char* _filepath, const char* _modelName, render::R_Model* model_)
		{
			//PERF_INIT()
			char filename[128];
			sprintf(filename, "%s/%s.gltf", _filepath, _modelName);
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
			for (size_t c = 0; c < scene->mNumCameras; c++)
			{
				fprintf(stdout, "Cameras %s", scene->mCameras[c]->mName);
			}
			for (size_t l = 0; l < scene->mNumLights; l++)
			{
				fprintf(stdout, "Light %s", scene->mLights[l]->mName);
			}
		}
		cgltf_data* read_glTF(const char* _filepath, const char* _modelName, render::R_Model* tempModel_)
		{
			cgltf_options options {};
			char filename[128];
			sprintf(filename, "%s/%s.gltf", _filepath, _modelName);
			cgltf_data* modelData = NULL;
			cgltf_parse_file(&options, filename, &modelData);
			if(modelData == NULL) return nullptr;
			cgltf_load_buffers(&options, modelData, _filepath);
			if(cgltf_validate(modelData) == cgltf_result_success)
			{
				printf("\nModel file validated (nodes %d)\n", static_cast<int>(modelData->nodes_count));
				//  MATERIALS
				int materialID = 0;
				if(modelData->materials_count > 0)
				{
					for (int i = 0 ; i < modelData->materials_count; i++) 
					{
						auto material = modelData->materials[i];
						materialID = i;
						if (tempModel_->m_Materials[materialID] == nullptr)
						{
							tempModel_->m_Materials[materialID] = new render::R_Material();
							tempModel->m_Materials[materialID]->material = new render::MaterialInstance();
							tempModel_->m_Materials[materialID]->material->pipeline._buildPipeline();
							fprintf(stderr, "Material %d: %s\n", materialID, material.name);
							/*for(int t= 0; t < 8; t++)
							{
								tempModel_->m_Materials[materialID]->textures[t] = new render::Texture();
							}*/
							/*if(material.normal_texture.texture != nullptr)
							{
								std::string pathTexture = std::string(material.normal_texture.texture->image->uri);
								tempModel_->m_Materials[materialID]->textures[2] = new render::Texture(pathTexture);
							}
							if(material.specular.specular_texture.texture != nullptr)
							{
								std::string pathTexture = std::string(material.specular.specular_texture.texture->image->uri);
								tempModel_->m_Materials[materialID]->textures[3] = new render::Texture(pathTexture);
							}*/

						}
					}
				}
				else
				{
					tempModel_->m_Materials[materialID] = new render::R_Material();
					#ifdef WIN32
					__debugbreak();
					#else
						raise(SIGTRAP);
					#endif
					for (int t = 0; t < MAX_TEXTURES; t++)
					{
						tempModel_->m_Materials[materialID]->textures[t] = new render::Texture("");
					}
				}

				for (cgltf_size n = 0; n < modelData->nodes_count; ++n)
				{
					cgltf_mesh* mesh = modelData->nodes[n].mesh;
					auto camera = modelData->nodes[n].camera;
					if(mesh)
					{
						VKR::render::VKRenderable* tempMesh = new VKR::render::VKRenderable();
						sprintf(tempMesh->m_Id, "%s_%d", mesh->name ? mesh->name : "mesh", n);
						tempMesh->m_Pos = glm::vec3(modelData->nodes[n].translation[0],
											modelData->nodes[n].translation[1],
											modelData->nodes[n].translation[2]);
						tempMesh->m_ModelMatrix = glm::mat4(
							modelData->nodes[n].matrix[0], modelData->nodes[n].matrix[1], modelData->nodes[n].matrix[2], modelData->nodes[n].matrix[3],
							modelData->nodes[n].matrix[4], modelData->nodes[n].matrix[5], modelData->nodes[n].matrix[6], modelData->nodes[n].matrix[7],
							modelData->nodes[n].matrix[8], modelData->nodes[n].matrix[9], modelData->nodes[n].matrix[10], modelData->nodes[n].matrix[11],
							modelData->nodes[n].matrix[12], modelData->nodes[n].matrix[13], modelData->nodes[n].matrix[14], modelData->nodes[n].matrix[15]
							);
						tempMesh->m_ModelMatrix = glm::translate(tempMesh->m_ModelMatrix, tempMesh->m_Pos);
						
						tempMesh->m_Rotation = glm::vec3(modelData->nodes[n].rotation[0],
											modelData->nodes[n].rotation[1],
											modelData->nodes[n].rotation[2]);
						//tempMesh->m_ModelMatrix = glm::rotate<float>(tempMesh->m_ModelMatrix, modelData->nodes[n].rotation[3], tempMesh->m_Rotation);
						tempMesh->m_Scale = glm::vec3(modelData->nodes[n].scale[0],
											modelData->nodes[n].scale[1],
											modelData->nodes[n].scale[2]);
						tempMesh->m_ModelMatrix = glm::scale(tempMesh->m_ModelMatrix, tempMesh->m_Scale);

						for(cgltf_size p = 0; p < mesh->primitives_count; p++)
						{
							printf("\tPrimitive %zd\n", p);
							for(cgltf_size a = 0; a < mesh->primitives[p].attributes_count; a++)
							{
#pragma region PBR_material
								auto meshMaterial = mesh->primitives[p].material;
								/*
								 *BASE_COLOR
								 *DIFFUSE_ROUGHNESS,
								 *SPECULAR,
								 *AMBIENT,
								 *NORMALS,
								 *AMBIENT_OCCLUSION,
								 *EMISSION_COLOR,
								 *LIGHTMAP
								 *
								 */
								tempMesh->m_Material = static_cast<uint32_t>(cgltf_material_index(modelData, meshMaterial));
								if (meshMaterial->pbr_metallic_roughness.base_color_texture.texture != nullptr)
								{
									std::string pathTexture = std::string(meshMaterial->pbr_metallic_roughness.base_color_texture.texture->image->uri);
									fprintf(stderr, "%s\n", (_filepath + pathTexture).c_str());
									tempModel_->m_Materials[tempMesh->m_Material]->textures[0] = new render::Texture(_filepath + pathTexture);
								}
								if(meshMaterial && meshMaterial->pbr_metallic_roughness.base_color_factor != nullptr)
								{
									tempMesh->metallic_roughness.base_color = glm::vec4(
										meshMaterial->pbr_metallic_roughness.base_color_factor[0],
										meshMaterial->pbr_metallic_roughness.base_color_factor[1],
										meshMaterial->pbr_metallic_roughness.base_color_factor[2],
										meshMaterial->pbr_metallic_roughness.base_color_factor[3]
									);
									tempMesh->metallic_roughness.pbr_factors[0] = meshMaterial->pbr_metallic_roughness.metallic_factor;
									tempMesh->metallic_roughness.pbr_factors[1] = meshMaterial->pbr_metallic_roughness.roughness_factor;
									// Additional textures PBR Material
									if (meshMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture != nullptr)
									{
										std::string pathTexture = std::string(meshMaterial->pbr_metallic_roughness.metallic_roughness_texture.texture->image->uri);
										//material.pbr_metallic_roughness.metallic_roughness_texturematerial.
										fprintf(stderr, "%s", (_filepath + pathTexture).c_str());
										tempMesh->metallic_roughness.pbr_texture = new render::Texture(_filepath + pathTexture);

									}
								}
#pragma endregion
								auto accessor = mesh->primitives[p].attributes[a].data;
								Vertex3D tempVertex;
								if(cgltf_attribute_type_position == mesh->primitives[p].attributes[a].type)
								{
									if(accessor->type == cgltf_type_vec3)
									{
										fprintf(stderr, "vertex count %llu\n", accessor->count);
										float* vertices = new float[3*accessor->count];
										int n = 0; 
										float *buffer = (float *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(float) + accessor->offset/sizeof(float); 
										for (unsigned int k = 0; k < accessor->count; k++) 
										{
											float tempVrtx[3];
											for (int l = 0; l < 3; l++) 
											{
												vertices[3*k + l] = buffer[n + l];
												tempVrtx[l] = buffer[n + l];
											}
											tempVertex.m_Pos = glm::vec3(tempVrtx[0], tempVrtx[1], tempVrtx[2]);
											if(tempMesh->m_Vertices.size() <= k)
												tempMesh->m_Vertices.push_back(tempVertex);
											else
												tempMesh->m_Vertices[k].m_Pos = tempVertex.m_Pos;
											n += (int)(accessor->stride/sizeof(float));
										}
									}
								}
								else if(cgltf_attribute_type_normal == mesh->primitives[p].attributes[a].type)
								{
									if(accessor->type == cgltf_type_vec3)
									{
										fprintf(stderr, "normal count %llu\n", accessor->count);
										float* values = new float[3*accessor->count];
										int n = 0; 
										float *buffer = (float *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(float) + accessor->offset/sizeof(float); 
										for (unsigned int k = 0; k < accessor->count; k++) 
										{
											float tempNormal[3];
											for (int l = 0; l < 3; l++) 
											{
												values[3*k + l] = buffer[n + l];
												tempNormal[l] = buffer[n + l];
											}
											tempVertex.m_Normal = glm::vec3(tempNormal[0], tempNormal[1], tempNormal[2]);
											if(tempMesh->m_Vertices.size() <= k)
												tempMesh->m_Vertices.push_back(tempVertex);
											else
												tempMesh->m_Vertices[k].m_Normal = tempVertex.m_Normal;
											n += (int)(accessor->stride/sizeof(float));
										}
									}
								}
								else if(cgltf_attribute_type_tangent == mesh->primitives[p].attributes[a].type)
								{

								}
								else if(cgltf_attribute_type_texcoord == mesh->primitives[p].attributes[a].type)
								{
									if(accessor->type == cgltf_type_vec2)
									{
										fprintf(stderr, "Texcoord count %llu\n", accessor->count);
										float* values = new float[2*accessor->count];
										int n = 0; 
										float *buffer = (float *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(float) + accessor->offset/sizeof(float); 
										for (unsigned int k = 0; k < accessor->count; k++) 
										{
											float tempTexCoord[2];
											for (int l = 0; l < 2; l++) 
											{
												values[2*k + l] = buffer[n + l];
												tempTexCoord[l] = buffer[n + l];
											}
											tempVertex.m_TexCoord = glm::vec2(tempTexCoord[0], tempTexCoord[1]);
											if(tempMesh->m_Vertices.size() <= k)
												tempMesh->m_Vertices.push_back(tempVertex);
											else
												tempMesh->m_Vertices[k].m_TexCoord = tempVertex.m_TexCoord;
											n += (int)(accessor->stride/sizeof(float));
										}
									}
								}
								else 
								{
									fprintf(stderr,  "Deshecho: %s\n", mesh->primitives[p].attributes[a].name);
								}
							}
							if(mesh->primitives[p].indices != NULL)
							{
								int n = 0; 
								auto accessor = mesh->primitives[p].indices;
								unsigned short index;
								unsigned short *buffer = (unsigned short *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(unsigned short) + accessor->offset/sizeof(unsigned short); 
								for (unsigned int k = 0; k < accessor->count; k++) 
								{
									index = buffer[n];
									// fprintf(stderr, "Index %d: ",index );
									tempMesh->m_Indices.push_back(index);
									n += (int)(accessor->stride/sizeof(unsigned short));
								}
							}
						}
						tempModel_->m_Meshes.push_back(tempMesh);
					}
				}
			}
			else
#ifdef WIN32
				__debugbreak();
#else
				raise(SIGTRAP);
#endif
			return modelData;
		}
#if 0
		cgltf_data* read_glTF_DBG(const char* _filepath, const char* _modelName, render::R_Model* tempModel_)
		{
			auto result =  read_glTF(_filepath, _modelName, tempModel_);
			// TODO cast R_Model to R_DbgModel
			//tempModel_->m_Rotation = _model->m_RotAngle;
			tempModel_->m_ModelMatrix = glm::mat4(1.f);
			//tempModel_->m_Pos = _model->m_Pos;
			//tempModel_->m_Material = new render::R_Material();
			//tempModel_->m_Material->m_Texture = new render::Texture(_model->m_Materials[0]->m_TextureAmbient->m_Path);
			for (auto& mesh : _model->m_Meshes) 
			{
				for (auto& vertex : mesh->m_Vertices) 
				{
					tempModel_->m_Vertices.push_back({vertex.m_Pos,{0.4f, 0.f, 1.f}});
				}
				tempModel_->m_Indices.insert(tempModel_->m_Indices.end(), mesh->m_Indices.begin(), mesh->m_Indices.end());
			}
			return result;
		}
#endif
	}
}
