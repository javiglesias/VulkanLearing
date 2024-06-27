#include "VKRScene.h"

#include "Types.h"

namespace VKR
{
	namespace render
	{
		static Scene g_MainScene;
		Scene& GetVKMainScene()
		{
			return g_MainScene;
		}

		R_Model* Scene::LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
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
			return tempModel;
		}
		void Scene::LoadStaticModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			m_StaticModels.push_back(LoadModel(_filepath, _modelName, _position, _scale, _customTexture));
		}

		void Scene::LoadCubemapModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			m_Cubemap->m_gltf = LoadModel(_filepath, _modelName, _position, _scale, _customTexture);
		}

		void Scene::CreateDebugModel(PRIMITIVE _type)
		{
			auto tempModel = new R_DbgModel(_type);
			tempModel->m_Material.m_Texture = new Texture();
			m_DbgModels.push_back(tempModel);
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

		void Scene::ShadowPass(VKBackend* _backend, int _CurrentFrame)
		{
			//glm::mat4 shadowProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
			glm::mat4 shadowProjMat = glm::ortho<float>(-g_LightRight, g_LightRight, -g_LightUp, g_LightUp, -g_LightDepth, g_LightDepth);
			shadowProjMat[1][1] *= -1;
			glm::mat4 lightViewMat = glm::lookAt(m_LightPos, m_LightPos+m_LightCenter, m_LightUp);
			glm::mat4 lightModelMat = glm::mat4(1.f);
			auto renderContext = GetVKContext();
			/// Shadow Pass
			{
				// Clear Color
				VkClearValue clearValue;
				clearValue.depthStencil = { 1.0f, 0 };
				// Update Uniform buffers
				ShadowUniformBufferObject ubo{};
				ubo.view = lightViewMat;
				ubo.projection = shadowProjMat;
				ubo.depthMVP = shadowProjMat * lightViewMat * lightModelMat;
				memcpy(_backend->m_ShadowUniformBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));

				VkRenderPassBeginInfo renderPassInfo{};
				renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassInfo.renderPass = renderContext.m_ShadowPass->m_Pass;
				renderPassInfo.framebuffer = _backend->m_ShadowFramebuffer;
				renderPassInfo.renderArea.offset = { 0,0 };
				renderPassInfo.renderArea.extent = _backend->m_CurrentExtent;
				renderPassInfo.clearValueCount = 1;
				renderPassInfo.pClearValues = &clearValue;
				vkCmdBeginRenderPass(_backend->m_CommandBuffer[_CurrentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

				vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowRender->m_Pipeline);
				vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
				vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);

				auto dynamicAlignment = sizeof(DynamicBufferObject);
				if (renderContext.m_GpuInfo.minUniformBufferOffsetAlignment > 0)
				{
					dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1) & ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
				}
				uint32_t count = 0;
				// Sombras(Depth Pass)
				for (auto& model : m_StaticModels)
				{
					DynamicBufferObject dynO{};
					dynO.model = glm::mat4(1.0f);
					uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
					// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
					memcpy((char*)_backend->m_ShadowDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
					vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ShadowRender->m_PipelineLayout,
						0, 1, &_backend->m_ShadowMat->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
					for (auto& mesh : model->m_Meshes)
					{
						// Update Uniform buffers

						VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
						VkDeviceSize offsets[] = { 0 };
						vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
						vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
						// Draw Loop
						if (m_IndexedRender && mesh->m_Indices.size() > 0)
						{
							vkCmdDrawIndexed(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
						}
						else
						{
							vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_Vertices.size(), 1, 0, 0);
						}
						// Flush to make changes visible to the host
						VkMappedMemoryRange mappedMemoryRange{};
						mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
						mappedMemoryRange.memory = _backend->m_ShadowDynamicBuffersMemory[_CurrentFrame];
						mappedMemoryRange.size = sizeof(dynO);
						vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
					}
					++count;
				}
				vkCmdEndRenderPass(_backend->m_CommandBuffer[_CurrentFrame]);
			}
		}

		void Scene::GeometryPass(VKBackend* _backend, int _CurrentFrame)
		{

		}

		void Scene::ReloadShaders(VKBackend* _backend)
		{
			/*m_CubemapRender->Initialize();
			m_CubemapRender->CreatePipelineLayoutSetup(&_backend->m_CurrentExtent, &_backend->m_Viewport, &_backend->m_Scissor);
			m_CubemapRender->CreatePipelineLayout();
			m_CubemapRender->CreatePipeline(g_context.m_RenderPass->m_Pass);
			m_CubemapRender->CleanShaderModules();*/
			
			if(m_GraphicsRender->m_VertShader->GLSLCompile(true) &&
				m_GraphicsRender->m_FragShader->GLSLCompile(true))
			{
				m_GraphicsRender->Initialize();
				m_GraphicsRender->CreatePipelineLayoutSetup(&_backend->m_CurrentExtent, &_backend->m_Viewport, &_backend->m_Scissor);
				m_GraphicsRender->CreatePipelineLayout();
				m_GraphicsRender->CreatePipeline(g_context.m_RenderPass->m_Pass);
				m_GraphicsRender->CleanShaderModules();
			}
		}

		void Scene::DrawScene(VKBackend* _backend, int _CurrentFrame)
		{
			auto renderContext = GetVKContext();

			GeometryPass(_backend, _CurrentFrame);
			ShadowPass(_backend, _CurrentFrame);
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(m_CameraFOV), m_Width / (float)m_Height, zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			glm::mat4 lightViewMat = glm::lookAt(m_LightPos, m_LightPos + m_LightForward, m_LightUp);
			glm::mat4 lightProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
			glm::mat4 viewMat = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			/// Render Pass
			// Update Uniform buffers
			UniformBufferObject ubo{};
			ubo.view = viewMat;
			ubo.projection = projMat;
			ubo.lightView = lightViewMat;
			ubo.lightProj = lightProjMat;
			ubo.cameraPosition = m_CameraPos;
			ubo.lightPosition = m_LightPos;
			ubo.lightColor = m_LightColor;
			memcpy(_backend->m_Uniform_SBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));
			_backend->BeginRenderPass(_CurrentFrame);
			// Drawing Commands
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsRender->m_Pipeline);
			// REFRESH RENDER MODE FUNCTIONS
			vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
			vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);
			uint32_t count = 0;
			// Render Pass
			for (auto& model : m_StaticModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = model->m_ModelMatrix;
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				dynO.model = glm::scale(dynO.model, model->m_Scale);
				dynO.model = glm::rotate(dynO.model, model->m_RotGRAD, model->m_RotAngle);
				dynO.lightOpts.x = g_ShadowBias;
				dynO.lightOpts.y = model->m_ProjectShadow; // project shadow
				dynO.lightOpts.z = g_MipLevel;
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_DynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers

					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };

					vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsRender->m_PipelineLayout, 0, 1,
						&model->m_Materials[mesh->m_Material]->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
					vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
					vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
					// Draw Loop
					if (m_IndexedRender && mesh->m_Indices.size() > 0)
					{
						vkCmdDrawIndexed(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_Vertices.size(), 1, 0, 0);
					}
					// Flush to make changes visible to the host
					VkMappedMemoryRange mappedMemoryRange{};
					mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
					mappedMemoryRange.memory = _backend->m_DynamicBuffersMemory[_CurrentFrame];
					mappedMemoryRange.size = sizeof(dynO);
					vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
				}
				++count;
			}
			// DEBUG Render
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_Pipeline);
			//Debug
			DebugUniformBufferObject dubo{};
			dubo.view = viewMat;
			dubo.projection = projMat;
			memcpy(_backend->m_DbgUniformBuffersMapped[_CurrentFrame], &dubo, sizeof(dubo));
			
			int debugCount = 0;
			for (auto& model : m_DbgModels)
			{
				DynamicBufferObject dynO{};
				dynO.model = model->m_ModelMatrix;
				dynO.model = glm::translate(dynO.model, m_LightPos);
				dynO.model = glm::scale(dynO.model, glm::vec3(1.f) * g_debugScale);
				dynO.model = glm::rotate<float>(dynO.model, g_Rotation, m_Rotation);

				uint32_t dynamicOffset = debugCount * static_cast<uint32_t>(dynamicAlignment);
				VkBuffer vertesBuffers[] = { model->m_VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_DbgDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_PipelineLayout, 0, 1, 
					&model->m_Material.m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
				vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
				vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], model->m_Vertices.size(), 1, 0, 0);
				// Flush to make changes visible to the host
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = _backend->m_DbgDynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
				++debugCount;
			}
			//_backend->EndRenderPass(_CurrentFrame);
			//DrawCubemapScene(_backend, _CurrentFrame, projMat, viewMat, static_cast<uint32_t>(dynamicAlignment));
		}

		void Scene::PrepareCubemapScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			/// N - Actualizar los DynamicDescriptorBuffers
			m_Cubemap->m_Material->PrepareMaterialToDraw(_backend);
			/// 5 - Crear buffers de vertices
			void* data;
			VkDeviceSize bufferSize = sizeof(m_Cubemap->m_Vertices[0]) * m_Cubemap->m_Vertices.size();
			// Stagin buffer
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
			vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, m_Cubemap->m_Vertices.data(), (size_t)bufferSize);
			vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
			CreateBuffer(bufferSize,
				VK_BUFFER_USAGE_TRANSFER_DST_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_SHARING_MODE_CONCURRENT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_Cubemap->m_VertexBuffer, m_Cubemap->m_VertexBufferMemory);
			CopyBuffer(m_Cubemap->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
			vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
			vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

			m_Cubemap->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_CubemapUniformBuffers, _backend->m_CubemapDynamicBuffers);

		}
		void Scene::DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment)
		{
			// Cubemap draw
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_Pipeline);
			auto renderContext = GetVKContext();
			constexpr int sizeCUBO = sizeof(CubemapUniformBufferObject);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(m_CameraFOV), m_Width / (float)m_Height, zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			glm::mat4 viewMat = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			CubemapUniformBufferObject cubo {};
			cubo.projection = projMat;
			cubo.view = viewMat;
			memcpy(_backend->m_CubemapUniformBuffersMapped[_CurrentFrame], &cubo, sizeof(cubo));
			constexpr int sizeDynO = sizeof(DynamicBufferObject);
			DynamicBufferObject dynO{};
			dynO.model = glm::mat4(1.f);
			dynO.model = glm::scale(dynO.model, glm::vec3(1.f) * g_cubemapDistance);
			uint32_t dynamicOffset = 0 * static_cast<uint32_t>(_dynamicAlignment);
			// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
			memcpy((char*)_backend->m_CubemapDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
			// Update Uniform buffers

			VkBuffer vertesBuffers[] = { m_Cubemap->m_VertexBuffer };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_CubemapRender->m_PipelineLayout, 0, 1,
				&m_Cubemap->m_Material->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
			vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
			for (auto& mesh : m_Cubemap->m_gltf->m_Meshes)
			{
				vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_Vertices.size(), 1, 0, 0);
			}
			// Flush to make changes visible to the host
			VkMappedMemoryRange mappedMemoryRange{};
			mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			mappedMemoryRange.memory = _backend->m_CubemapDynamicBuffersMemory[_CurrentFrame];
			mappedMemoryRange.size = sizeof(dynO);
			vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
		}
		void Scene::PrepareScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
			_backend->m_CurrentModelsToDraw = m_StaticModels.size();
			/// 1 - Actualizar los DynamicDescriptorBuffers
			_backend->GenerateBuffers();
			/*VkDeviceSize dynBufferSize = m_StaticModels.size() * sizeof(DynamicBufferObject);
			for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
			{
				CreateBuffer(dynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_backend->m_DynamicBuffers[i], _backend->m_DynamicBuffersMemory[i]);
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_DynamicBuffersMemory[i], 0,
					dynBufferSize, 0, &_backend->m_DynamicBuffersMapped[i]);
			}*/
			for (auto& model : m_StaticModels)
			{
				for (auto& mesh : model->m_Meshes)
				{

					model->m_Materials[mesh->m_Material]->PrepareMaterialToDraw(_backend);

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
						_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
					vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
					memcpy(data, mesh->m_Vertices.data(), (size_t)bufferSize);
					vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
					CreateBuffer(bufferSize,
						VK_BUFFER_USAGE_TRANSFER_DST_BIT |
						VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						mesh->m_VertexBuffer, mesh->m_VertexBufferMemory);
					CopyBuffer(mesh->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
					vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
					vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

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
							_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
						vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
						memcpy(data, mesh->m_Indices.data(), (size_t)bufferSize);
						vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
						CreateBuffer(bufferSize,
							VK_BUFFER_USAGE_TRANSFER_DST_BIT |
							VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
							VK_SHARING_MODE_CONCURRENT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							mesh->m_IndexBuffer, mesh->m_IndexBufferMemory);
						CopyBuffer(mesh->m_IndexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
						vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
						vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);
					}

					/// 8 - Actualizar Descrip Sets(UpdateDescriptorSet)
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageView = _backend->m_ShadowImageView;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImage = _backend->m_ShadowImage;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->tImageMem = _backend->m_ShadowImageMemory;
					model->m_Materials[mesh->m_Material]->m_TextureShadowMap->m_Sampler = _backend->m_ShadowImgSamp;
					model->m_Materials[mesh->m_Material]->UpdateDescriptorSet(renderContext.m_LogicDevice,
						_backend->m_UniformBuffers, _backend->m_DynamicBuffers);
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
		}
		void Scene::Init()
		{
			m_Cubemap = new R_Cubemap("resources/Textures/cubemaps/office.png");
		}
		void Scene::PrepareDebugScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
            _backend->m_CurrentDebugModelsToDraw = m_DbgModels.size();
			_backend->GenerateDBGBuffers();
			/*if (m_DbgModels.size() > 0)
			{
				m_DbgModels[0]->m_Pos = m_LightPos;

				/// N - Actualizar los DynamicDescriptorBuffers
				VkDeviceSize dynDbgBufferSize = m_DbgModels.size() * sizeof(DynamicBufferObject);
				for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
				{
					CreateBuffer(dynDbgBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_SHARING_MODE_CONCURRENT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
						VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						_backend->m_DbgDynamicBuffers[i], _backend->m_DbgDynamicBuffersMemory[i]);
					vkMapMemory(renderContext.m_LogicDevice, _backend->m_DbgDynamicBuffersMemory[i], 0,
						dynDbgBufferSize, 0, &_backend->m_DbgDynamicBuffersMapped[i]);
				}
			}*/
			for (auto& dbgModel : m_DbgModels)
			{
				dbgModel->m_Material.m_Texture->LoadTexture();
				/// 2 - Crear descriptor pool de materiales(CreateDescPool)`
				dbgModel->m_Material.CreateDescriptorPool(renderContext.m_LogicDevice);

				/// 3 - Crear Descriptor set de material(createMeshDescSet)
				dbgModel->m_Material.CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_DbgRender->m_DescSetLayout);
				/// 4 - Crear y transicionar texturas(CreateAndTransImage)
				dbgModel->m_Material.m_Texture->CreateAndTransitionImage(_backend->m_CommandPool);
				/// 5 - Crear buffers de vertices
				void* data;
				VkDeviceSize bufferSize = sizeof(dbgModel->m_Vertices[0]) * dbgModel->m_Vertices.size();
				// Stagin buffer
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
					VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					_backend->m_StagingBuffer, _backend->m_StaggingBufferMemory);
				vkMapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, 0, bufferSize, 0, &data);
				memcpy(data, dbgModel->m_Vertices.data(), (size_t)bufferSize);
				vkUnmapMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory);
				CreateBuffer(bufferSize,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_SHARING_MODE_CONCURRENT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					dbgModel->m_VertexBuffer, dbgModel->m_VertexBufferMemory);
				CopyBuffer(dbgModel->m_VertexBuffer, _backend->m_StagingBuffer, bufferSize, _backend->m_CommandPool);
				vkDestroyBuffer(renderContext.m_LogicDevice, _backend->m_StagingBuffer, nullptr);
				vkFreeMemory(renderContext.m_LogicDevice, _backend->m_StaggingBufferMemory, nullptr);

				dbgModel->m_Material.UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_DbgUniformBuffers, _backend->m_DbgDynamicBuffers);

			}
		}

		void Scene::Cleanup(VkDevice _LogicDevice)
		{
			printf("Scene Cleanup\n");
			vkDeviceWaitIdle(_LogicDevice);
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
			m_Cubemap->Cleanup(_LogicDevice);
		}
	}
}
