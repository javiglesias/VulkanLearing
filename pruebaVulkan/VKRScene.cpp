#include "VKRScene.h"

#include "Types.h"

namespace VKR
{
	namespace render
	{
		Scene::Scene() : m_State(INVALID)
		{}

		void Scene::LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
			printf("\nLoading %s\n", _modelName);
			const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
			if (!scene->HasMeshes())
				exit(-225);
			tempModel = new R_Model();
			//Process Node
			auto node = scene->mRootNode;
			ProcessModelNode(node, scene, _filepath, _customTexture);
			// Insert new static model
			sprintf(tempModel->m_Path, _filepath, 64);
			tempModel->m_Pos = _position;
			tempModel->m_Scale = _scale;
			m_StaticModels.push_back(tempModel);
			m_State = DIRTY;
		}

		void Scene::ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture)
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
				R_Mesh* tempMesh = new R_Mesh();
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
					// printf("\tNew Material %d\n", mesh->mMaterialIndex);
					tempModel->m_Materials[mesh->mMaterialIndex] = new R_Material();
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
					auto textureDiffuse = std::string(_filepath);
					textureDiffuse += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureDiffuse = new Texture(textureDiffuse);
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_SPECULAR, texIndex, &path);
					auto textureSpecular = std::string(_filepath);
					textureSpecular += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureSpecular = new Texture(textureSpecular);
					_scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_AMBIENT, texIndex, &path);
					auto textureAmbient = std::string(_filepath);
					textureAmbient += path.data;
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureAmbient = new Texture(textureAmbient);
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureShadowMap = new Texture();
				}
				else if (_customTexture != nullptr) // renemos que crear el modelo con textura custom
				{
					tempModel->m_Materials[mesh->mMaterialIndex] = new R_Material();
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureDiffuse = new Texture(std::string(_customTexture));
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureSpecular = new Texture(std::string(_customTexture));
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureAmbient = new Texture(std::string(_customTexture));
					tempModel->m_Materials[mesh->mMaterialIndex]->m_TextureShadowMap = new Texture();
				}
				tempMesh->m_Material = mesh->mMaterialIndex;
				//++m_TotalTextures;
				tempModel->m_Meshes.push_back(tempMesh);
			}
		}
		void Scene::Loop()
		{
			if (m_State == DIRTY)
				ManageModels();
		}

		void Scene::DrawScene()
		{
#if 0
			uint32_t count = 0;

			for (auto& model : m_StaticModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = glm::mat4(1.0f);
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				dynO.model = glm::scale(dynO.model, model->m_Scale);
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)m_ShadowDynamicBuffersMapped[_frameIdx] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowRender->m_PipelineLayout,
					0, 1, &m_ShadowMat->m_DescriptorSet[_frameIdx], 1, &dynamicOffset);
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers

					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
					vkCmdBindIndexBuffer(_commandBuffer, mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
					// Draw Loop
					if (m_IndexedRender && mesh->m_Indices.size() > 0)
					{
						vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_commandBuffer, mesh->m_Vertices.size(), 1, 0, 0);
					}
					// Flush to make changes visible to the host
					VkMappedMemoryRange mappedMemoryRange{};
					mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					mappedMemoryRange.memory = m_ShadowDynamicBuffersMemory[_frameIdx];
					mappedMemoryRange.size = sizeof(dynO);
					vkFlushMappedMemoryRanges(g_context.m_LogicDevice, 1, &mappedMemoryRange);
				}
				++count;
			}

			uint32_t count = 0;
			for (auto& model : m_StaticModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = glm::mat4(1.0f);
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				dynO.model = glm::scale(dynO.model, model->m_Scale);
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)m_DynamicBuffersMapped[_frameIdx] + dynamicOffset, &dynO, sizeof(dynO));
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers

					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };

					vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _renderer->m_PipelineLayout, 0, 1,
						&model->m_Materials[mesh->m_Material]->m_DescriptorSet[_frameIdx], 1, &dynamicOffset);
					vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
					vkCmdBindIndexBuffer(_commandBuffer, mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
					// Draw Loop
					if (m_IndexedRender && mesh->m_Indices.size() > 0)
					{
						vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_commandBuffer, mesh->m_Vertices.size(), 1, 0, 0);
					}
					// Flush to make changes visible to the host
					VkMappedMemoryRange mappedMemoryRange{};
					mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					mappedMemoryRange.memory = m_DynamicBuffersMemory[_frameIdx];
					mappedMemoryRange.size = sizeof(dynO);
					vkFlushMappedMemoryRanges(g_context.m_LogicDevice, 1, &mappedMemoryRange);
				}
				++count;
			}
			if (m_DebugRendering)
			{

				// DEBUG Render
				vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_Pipeline);
				//Debug
				DebugUniformBufferObject dubo{};
				dubo.view = viewMat;
				dubo.projection = projMat;

				memcpy(m_DbgUniformBuffersMapped[_imageIdx], &dubo, sizeof(dubo));
				// REFRESH RENDER MODE FUNCTIONS
				vkCmdSetViewport(_commandBuffer, 0, 1, &m_Viewport);
				vkCmdSetScissor(_commandBuffer, 0, 1, &m_Scissor);
				int debugCount = 0;
				m_DbgModels[0]->m_Pos = m_LightPos;
				for (auto& model : m_DbgModels)
				{
					DynamicBufferObject dynO{};
					dynO.model = glm::mat4(1.0f);
					dynO.model = glm::translate(dynO.model, model->m_Pos);
					uint32_t dynamicOffset = debugCount * static_cast<uint32_t>(dynamicAlignment);
					VkBuffer vertesBuffers[] = { model->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
					memcpy((char*)m_DbgDynamicBuffersMapped[_frameIdx] + dynamicOffset, &dynO, sizeof(dynO));
					vkCmdBindDescriptorSets(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_PipelineLayout, 0, 1, &model->m_Material.m_DescriptorSet[_frameIdx], 1, &dynamicOffset);
					vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertesBuffers, offsets);
					vkCmdDraw(_commandBuffer, model->m_Vertices.size(), 1, 0, 0);
					// Flush to make changes visible to the host
					VkMappedMemoryRange mappedMemoryRange{};
					mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					mappedMemoryRange.memory = m_DbgDynamicBuffersMemory[_frameIdx];
					mappedMemoryRange.size = sizeof(dynO);
					vkFlushMappedMemoryRanges(g_context.m_LogicDevice, 1, &mappedMemoryRange);
					++debugCount;
				}
			}
#endif
		}

		void Scene::ManageModels()
		{
			/// Flujo actual para renderizar un Modelo(gltf) :
			/// 1 - cargar modelo y materiales(LoadModel)
			/*LoadModel(modelPath, modelName, glm::vec3(0.f, 2.f, 0.f),
				glm::vec3(0.5f, 0.5f, 0.5f));*/
#if 0
			for (auto& model : m_StaticModels)
			{
				for (auto& mesh : model->m_Meshes)
				{
					/// 2 - Crear descriptor pool de materiales(CreateDescPool)
					model->m_Materials[mesh->m_Material]->CreateDescriptorPool(g_context.m_LogicDevice);
				
					/// 3 - Crear Descriptor set de material(createMeshDescSet)
					model->m_Materials[mesh->m_Material]->CreateMeshDescriptorSet(g_context.m_LogicDevice, m_GraphicsRender->m_DescSetLayout);

					/// 4 - Crear y transicionar texturas(CreateAndTransImage)
					CreateAndTransitionImage(model->m_Materials[mesh->m_Material]->m_TextureDiffuse);
					CreateAndTransitionImage(model->m_Materials[mesh->m_Material]->m_TextureSpecular);
					CreateAndTransitionImage(model->m_Materials[mesh->m_Material]->m_TextureAmbient);

					/// 5 - Crear buffers de vertices
					void* data;
					if (mesh->m_Vertices.size() <= 0)
					{
						fprintf(stderr, "There is no Triangles to inser on the buffer");
						exit(-57);
		}
					VkDeviceSize bufferSize = sizeof(mesh->m_Vertices[0]) * mesh->m_Vertices.size();
					// Stagin buffer
					CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						m_StagingBuffer, m_StaggingBufferMemory);
					vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
					memcpy(data, mesh->m_Vertices.data(), (size_t)bufferSize);
					vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
					CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mesh->m_VertexBuffer, mesh->m_VertexBufferMemory);
					CopyBuffer(mesh->m_VertexBuffer, m_StagingBuffer, bufferSize);
					vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
					vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);

					/// 6 - Crear Buffers de Indices
					if (mesh->m_Indices.size() > 0)
					{
						// Index buffer
						bufferSize = sizeof(mesh->m_Indices[0]) * mesh->m_Indices.size();
						// Stagin buffer
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
							VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
							m_StagingBuffer, m_StaggingBufferMemory);
						vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
						memcpy(data, mesh->m_Indices.data(), (size_t)bufferSize);
						vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							mesh->m_IndexBuffer, mesh->m_IndexBufferMemory);
						CopyBuffer(mesh->m_IndexBuffer, m_StagingBuffer, bufferSize);
						vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
						vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);
					}

					/// 8 - Actualizar Descrip Sets(UpdateDescriptorSet)
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageView = m_ShadowImageView;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImage = m_ShadowImage;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageMem = m_ShadowImageMemory;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->m_Sampler = m_ShadowImgSamp;
					model->m_Materials[mesh->m_Material]->UpdateDescriptorSet(g_context.m_LogicDevice, m_UniformBuffers, m_DynamicBuffers);
				}
			}

			/// 8 - (OPCIONAL)Reordenar modelos
			// Vamos a pre-ordenar los modelos para pintarlos segun el material.
			// BUBBLESORT de primeras, luego ya veremos, al ser tiempo pre-frameloop, no deberia importar.
			for (auto& model : m_StaticModels)
			{
				for (int i = 0; i < model->m_Meshes.size(); i++)
				{
					for (int j = 1; j < model->m_Meshes.size(); j++)
					{
						auto& mesh = model->m_Meshes[i];
						if (model->m_Meshes[j]->m_Material > model->m_Meshes[i]->m_Material)
						{
							auto tempMesh = model->m_Meshes[j];
							model->m_Meshes[j] = model->m_Meshes[i];
							model->m_Meshes[i] = tempMesh;
						}
					}
				}
			}
			/// N - Actualizar los DynamicDescriptorBuffers
			VkDeviceSize dynBufferSize = m_CurrentModelsToDraw * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DynamicBuffers[i], m_DynamicBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DynamicBuffersMemory[i], 0,
					dynBufferSize, 0, &m_DynamicBuffersMapped[i]);
			}

			for (auto& model : m_DbgModels)
			{
				/// 2 - Crear descriptor pool de materiales(CreateDescPool)
				model->m_Material.CreateDescriptorPool(g_context.m_LogicDevice);

				/// 3 - Crear Descriptor set de material(createMeshDescSet)
				model->m_Material.CreateMeshDescriptorSet(g_context.m_LogicDevice, m_DbgRender->m_DescSetLayout);

				/// 5 - Crear buffers de vertices
				void* data;
				if (model->m_Vertices.size() <= 0)
				{
					fprintf(stderr, "There is no Triangles to inser on the buffer");
					exit(-57);
				}
				VkDeviceSize bufferSize = sizeof(model->m_Vertices[0]) * model->m_Vertices.size();
				// Stagin buffer
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_StagingBuffer, m_StaggingBufferMemory);
				vkMapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, 0, bufferSize, 0, &data);
				memcpy(data, model->m_Vertices.data(), (size_t)bufferSize);
				vkUnmapMemory(g_context.m_LogicDevice, m_StaggingBufferMemory);
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					model->m_VertexBuffer, model->m_VertexBufferMemory);
				CopyBuffer(model->m_VertexBuffer, m_StagingBuffer, bufferSize);
				vkDestroyBuffer(g_context.m_LogicDevice, m_StagingBuffer, nullptr);
				vkFreeMemory(g_context.m_LogicDevice, m_StaggingBufferMemory, nullptr);

				model->m_Material.UpdateDescriptorSet(g_context.m_LogicDevice, m_DbgUniformBuffers, m_DbgDynamicBuffers);
			}

			/// N - Actualizar los DynamicDescriptorBuffers
			VkDeviceSize dynDbgBufferSize = m_CurrentDebugModelsToDraw * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					m_DbgDynamicBuffers[i], m_DbgDynamicBuffersMemory[i]);
				vkMapMemory(g_context.m_LogicDevice, m_DbgDynamicBuffersMemory[i], 0,
					dynDbgBufferSize, 0, &m_DbgDynamicBuffersMapped[i]);
			}
#endif
			/// 9 - DrawFrame con todos los modelos ya creados.
			m_State = READY;
		}

		void Scene::Cleanup(VkDevice _LogicDevice)
		{
			for (auto& model : m_StaticModels)
			{
				for (auto& [idx, mat] : model->m_Materials)
				{
					mat->Cleanup(_LogicDevice);
				}
				for (auto& mesh : model->m_Meshes)
				{
					mesh->Cleanup(_LogicDevice);
				}
			}
			for (auto& model : m_DbgModels)
			{
				model->Cleanup(_LogicDevice);
			}
		}
	}
}
