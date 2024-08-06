#include "gltfReader.h"
#include "../../dependencies/cgltf/cgltf.h"
#include <glm/vec3.hpp>
#include <signal.h>
#include "../core/Objects/VKRModel.h"

namespace VKR
{
	namespace filesystem
	{
		cgltf_data* read_glTF(const char* _filepath, const char* _modelName)
		{
			cgltf_options options {};
			char filename[41];
			sprintf(filename, "%s%s", _filepath, _modelName);
			cgltf_data* modelData = NULL;
			cgltf_parse_file(&options, filename, &modelData);
			cgltf_load_buffers(&options, modelData, _filepath);
			if(cgltf_validate(modelData) == cgltf_result_success)
			{
				printf("\nModel file validated");
					for (cgltf_size n = 0; n < modelData->meshes_count; ++n)
					{
						auto mesh = &modelData->meshes[n];
						if(mesh)
						{
							printf("\n\tMesh %zd", n);
							for(cgltf_size p = 0; p < mesh->primitives_count; p++)
							{
								printf("\n\tPrimitive %zd", p);
								auto idxAccessor = mesh->primitives[p].indices;
								auto trigCount = idxAccessor->count/3;
								unsigned int indices[idxAccessor->count];
								cgltf_accessor_unpack_indices(mesh->primitives[p].indices, &indices,
									sizeof(unsigned int), idxAccessor->count);
								VKR::render::R_Mesh* tempMesh = new VKR::render::R_Mesh();
								for(cgltf_size i = 0; i < idxAccessor->count; i++)
								{
									tempMesh->m_Indices.push_back(indices[i]);
								}
								for(cgltf_size a = 0; a < mesh->primitives[p].attributes_count; a++)
								{
									if(cgltf_attribute_type_position == mesh->primitives[p].attributes[a].type)
									{
										auto accessor = mesh->primitives[p].attributes[a].data;
										auto index 	  = mesh->primitives[p].attributes[a].index;
										cgltf_size element_size = cgltf_calc_size(accessor->type, accessor->component_type);
										float position[element_size];
										if(cgltf_accessor_unpack_floats(accessor, position, element_size))
											for(size_t pos = 0; pos < element_size/3; pos++)
											{
												int idx = 3*pos;
												printf("\nVertex %zd: %f, %f, %f", p, position[idx], position[idx+1], position[idx+2]);
												tempMesh->m_Vertices.push_back(Vertex3D{glm::vec3(position[idx], position[idx+1], position[idx+2])});
											}
										else
										{
											printf("Error Reading Floats gltf");
										}
									}
								}
							}
						}
					}
				}
			else
				raise(SIGTRAP);
			return modelData;
		}
	}
}