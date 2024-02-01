#define VK_CHECK(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false);}

#define VK_CHECK_RET(_value) \
	if(_value != VK_SUCCESS) \
	{VK_ASSERT(false); return _value;}

#define CHECK(_expression) \
	exit(-96);

inline static void VK_ASSERT(bool _check)
{
	if(_check) exit(-69);
}


#if 0
	cgltf_options options {};
	cgltf_parse_file(&options, filename, &modelData);
	cgltf_data* modelData = NULL;
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
						for(cgltf_size i = 0; i < idxAccessor->count; i++)
						{
							m_Indices.push_back(indices[i]);
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
										m_ModelTriangles.push_back({glm::vec3(position[idx], position[idx+1], position[idx+2]), {1.0f, 0.0f, 0.0f}});
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
		exit(-88);
	return modelData;
#endif
