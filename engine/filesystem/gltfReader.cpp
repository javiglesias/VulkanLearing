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
			char filename[41];
			sprintf(filename, "%s%s", _filepath, _modelName);
			cgltf_data* modelData = NULL;
			cgltf_parse_file(&options, filename, &modelData);
			cgltf_load_buffers(&options, modelData, _filepath);
			if(cgltf_validate(modelData) == cgltf_result_success)
			{
				printf("\nModel file validated (nodes %ld)", modelData->nodes_count);
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
										// const float* position_data = (float *)accessor->buffer_view->buffer->data;
										// fprintf(stderr, "position %f,%f,%f\n", position_data[0], position_data[1], position_data[2]);
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
						//  TODO MATERIALS
							auto materialID = cgltf_material_index(modelData, mesh->primitives[p].material);
							tempMesh->m_Material = materialID;
							tempModel_->m_Materials[materialID] = new render::R_Material();
							fprintf(stderr, "Material %ld: %s\n", materialID, mesh->primitives[p].material->name);
							// std::string pathTexture = std::string(mesh->primitives[p].material->normal_texture.texture->image->uri);
							tempModel_->m_Materials[materialID]->m_TextureDiffuse = new render::Texture();
							tempModel_->m_Materials[materialID]->m_TextureSpecular = new render::Texture();
							tempModel_->m_Materials[materialID]->m_TextureAmbient = new render::Texture();
							tempModel_->m_Materials[materialID]->m_TextureShadowMap = new render::Texture();
						}
						tempModel_->m_Meshes.push_back(tempMesh);
					}
				}
			}
			else
				raise(SIGTRAP);
			return modelData;
		}
	}
}