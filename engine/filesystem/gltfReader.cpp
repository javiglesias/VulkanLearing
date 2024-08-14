#include "gltfReader.h"
#include "../../dependencies/cgltf/cgltf.h"
#include <cstddef>
#include <glm/vec3.hpp>
#include <signal.h>
#include <string>
#include "../core/Objects/VKRModel.h"

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
				printf("\nModel file validated (nodes %ld)", modelData->nodes_count);
				//  MATERIALS
				cgltf_size materialID = 0;
				if(modelData->materials_count > 0)
				{
					for (int i = 0 ; i < modelData->materials_count; i++) 
					{
						auto material = modelData->materials[i];
						materialID = i;
						if (tempModel_->m_Materials[materialID] == nullptr)
						{
							tempModel_->m_Materials[materialID] = new render::R_Material();
							fprintf(stderr, "Material %ld: %s\n", materialID, material.name);
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
					}
				}
				else
				{
					tempModel_->m_Materials[materialID] = new render::R_Material();
					tempModel_->m_Materials[materialID]->m_TextureSpecular = new render::Texture();
					tempModel_->m_Materials[materialID]->m_TextureDiffuse = new render::Texture();
					tempModel_->m_Materials[materialID]->m_TextureAmbient = new render::Texture();
				}
				tempModel_->m_Materials[materialID]->m_TextureShadowMap = new render::Texture();

				for (cgltf_size n = 0; n < modelData->nodes_count; ++n)
				{
					auto mesh = modelData->nodes[n].mesh;
					auto camera = modelData->nodes[n].camera;
					if(mesh)
					{
						VKR::render::R_Mesh* tempMesh = new VKR::render::R_Mesh();
						printf("\n\tMesh %zd %s (%ld)\n", n, mesh->name, mesh->primitives_count);
						for(cgltf_size p = 0; p < mesh->primitives_count; p++)
						{
							printf("\tPrimitive %zd\n", p);
							for(cgltf_size a = 0; a < mesh->primitives[p].attributes_count; a++)
							{
								if(cgltf_attribute_type_position == mesh->primitives[p].attributes[a].type)
								{
									auto accessor = mesh->primitives[p].attributes[a].data;
									if(accessor->type == cgltf_type_vec3)
									{
										fprintf(stderr, "vertex count %ld\n", accessor->count);
										float vertices[3*accessor->count];
										int n = 0; 
										float *buffer = (float *)accessor->buffer_view->buffer->data + accessor->buffer_view->offset/sizeof(float) + accessor->offset/sizeof(float); 
										for (unsigned int k = 0; k < accessor->count; k++) 
										{
											// fprintf(stderr, "Vertex %d: ",k );
											float tempVrtx[3];
											for (int l = 0; l < 3; l++) 
											{
												vertices[3*k + l] = buffer[n + l];
												tempVrtx[l] = buffer[n + l];
												// fprintf(stderr, " %f ", buffer[n + l]);
											}
											tempMesh->m_Vertices.push_back(Vertex3D{glm::vec3(tempVrtx[0], tempVrtx[1], tempVrtx[2])});
											n += (int)(accessor->stride/sizeof(float));
											// fprintf(stderr, "\n");
										}
									}
								}
								else if(cgltf_attribute_type_normal == mesh->primitives[p].attributes[a].type)
								{	

								}
								else if(cgltf_attribute_type_tangent == mesh->primitives[p].attributes[a].type)
								{

								}
								else if(cgltf_attribute_type_texcoord == mesh->primitives[p].attributes[a].type)
								{

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
				raise(SIGTRAP);
			return modelData;
		}
		cgltf_data* read_glTF_DBG(const char* _filepath, const char* _modelName, render::R_DbgModel* tempModel_)
		{
			render::R_Model* _model = new render::R_Model();
			auto result =  read_glTF(_filepath, _modelName, _model);
			// TODO cast R_Model to R_DbgModel
			tempModel_->m_Rotation = _model->m_RotAngle;
			tempModel_->m_ModelMatrix = _model->m_ModelMatrix;
			tempModel_->m_Pos = _model->m_Pos;
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