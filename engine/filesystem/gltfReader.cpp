#include "gltfReader.h"
#include "../../dependencies/cgltf/cgltf.h"
#include <cstddef>
#include <glm/vec3.hpp>
#include <signal.h>
#include <string>
#include "../core/Objects/VKRModel.h"
#include "../core/Materials/VKRTexture.h"
#include "../video/VKRenderable.h"

namespace VKR
{
	namespace filesystem
	{
		cgltf_data* read_glTF(const char* _filepath, const char* _modelName, render::R_Model* tempModel_)
		{
			cgltf_options options {};
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
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
							fprintf(stderr, "Material %d: %s\n", materialID, material.name);
							tempModel_->m_Materials[materialID]->m_TextureSpecular = new render::Texture();
							tempModel_->m_Materials[materialID]->m_TextureDiffuse = new render::Texture();
							tempModel_->m_Materials[materialID]->m_TextureAmbient = new render::Texture();
							if(material.normal_texture.texture != nullptr)
							{
								std::string pathTexture = std::string(material.normal_texture.texture->image->uri);
								tempModel_->m_Materials[materialID]->m_TextureDiffuse->m_Path =  _filepath + pathTexture;
							}
							if(material.specular.specular_texture.texture != nullptr)
							{
								std::string pathTexture = std::string(material.specular.specular_texture.texture->image->uri);
								tempModel_->m_Materials[materialID]->m_TextureSpecular->m_Path = _filepath + pathTexture;
							}
							if(material.pbr_metallic_roughness.base_color_texture.texture != nullptr)
							{
								std::string pathTexture = std::string(material.pbr_metallic_roughness.base_color_texture.texture->image->uri);
								//material.pbr_metallic_roughness.metallic_roughness_texturematerial.
								fprintf(stderr, "%s",  (_filepath + pathTexture).c_str());
								tempModel_->m_Materials[materialID]->m_TextureAmbient->m_Path = _filepath + pathTexture;
								
							}
						}
						tempModel_->m_Materials[materialID]->m_TextureEmissive = new render::Texture();
						tempModel_->m_Materials[materialID]->m_TextureMetallicRoughness = new render::Texture();
						tempModel_->m_Materials[materialID]->m_TextureOcclusion = new render::Texture();
						tempModel_->m_Materials[materialID]->m_TextureNormal = new render::Texture();
						tempModel_->m_Materials[materialID]->m_TextureShadowMap = new render::Texture();
					}
				}
				else
				{
					tempModel_->m_Materials[materialID] = new render::R_Material();
					tempModel_->m_Materials[materialID]->m_TextureSpecular = new render::Texture();
					tempModel_->m_Materials[materialID]->m_TextureDiffuse = new render::Texture();
					tempModel_->m_Materials[materialID]->m_TextureAmbient = new render::Texture();
				}

				for (cgltf_size n = 0; n < modelData->nodes_count; ++n)
				{
					auto mesh = modelData->nodes[n].mesh;
					auto camera = modelData->nodes[n].camera;
					if(mesh)
					{
						VKR::render::VKRenderable* tempMesh = new VKR::render::VKRenderable();
						tempMesh->m_ModelMatrix = glm::mat4(1.f);
						//modelData->nodes[n].translation
						tempMesh->m_Pos = glm::vec3(modelData->nodes[n].translation[0],
											modelData->nodes[n].translation[1],
											modelData->nodes[n].translation[2]);
						tempMesh->m_ModelMatrix = glm::translate(tempMesh->m_ModelMatrix, tempMesh->m_Pos);
						//modelData->nodes[n].rotation
						/*tempMesh->m_Rotation = glm::vec3(modelData->nodes[n].rotation[0],
											modelData->nodes[n].rotation[1],
											modelData->nodes[n].rotation[2]);*/
						//modelData->nodes[n].scale
						tempMesh->m_Scale = glm::vec3(modelData->nodes[n].scale[0],
											modelData->nodes[n].scale[1],
											modelData->nodes[n].scale[2]);
						tempMesh->m_ModelMatrix  = glm::scale(tempMesh->m_ModelMatrix, tempMesh->m_Scale);
						for(cgltf_size p = 0; p < mesh->primitives_count; p++)
						{
							printf("\tPrimitive %zd\n", p);
							for(cgltf_size a = 0; a < mesh->primitives[p].attributes_count; a++)
							{
								tempMesh->m_Material = static_cast<uint32_t>(cgltf_material_index(modelData, mesh->primitives[p].material));
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
		cgltf_data* read_glTF_DBG(const char* _filepath, const char* _modelName, render::R_DbgModel* tempModel_)
		{
			render::R_Model* _model = new render::R_Model();
			auto result =  read_glTF(_filepath, _modelName, _model);
			// TODO cast R_Model to R_DbgModel
			//tempModel_->m_Rotation = _model->m_RotAngle;
			tempModel_->m_ModelMatrix = glm::mat4(1.f);
			//tempModel_->m_Pos = _model->m_Pos;
			tempModel_->m_Material = new render::R_DbgMaterial();
			tempModel_->m_Material->m_Texture = new render::Texture(_model->m_Materials[0]->m_TextureAmbient->m_Path);
			for (auto& mesh : _model->m_Meshes) 
			{
				for (auto& vertex : mesh->m_Vertices) 
				{
					tempModel_->m_Vertices.push_back({vertex.m_Pos,{0.4f, 0.f, 1.f}});
				}
			}
			return result;
		}
	}
}