#include "VKRScene.h"
#include "../perfmon/Custom.h"
#include "../video/Types.h"
#include "Objects/VKRCubemap.h"
#include "../filesystem/gltfReader.h"
#include "Objects/VKRLight.h"
#include "Objects/VKRModel.h"
#include <cstddef>
#include "../filesystem/ResourceManager.h"

namespace VKR
{
	namespace render
	{
		static Scene g_MainScene;
		Scene& GetVKMainScene()
		{
			return g_MainScene;
		}
		R_Model* tempModel;
		void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture)
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
		void LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			PERF_INIT()
			char filename[128];
			sprintf(filename, "%s%s", _filepath, _modelName);
			printf("\nLoading %s\n", _modelName);
			const aiScene* scene = aiImportFile(filename, aiProcess_Triangulate);
			if (!scene || !scene->HasMeshes())
				exit(-225);
			tempModel = new R_Model();
			//Process Node
			auto node = scene->mRootNode;
			ProcessModelNode(node, scene, _filepath, _customTexture);
			// Insert new static model
			sprintf(tempModel->m_Path, _filepath, 64);
			tempModel->m_Pos = _position;
			tempModel->m_Scale = _scale;
			PERF_END("LOAD MODEL")
			m_StaticModels[m_CurrentStaticModels] = tempModel;
			m_CurrentStaticModels++;
		}
		void DispatchRMThread()
		{
			const char* _path = "resources/models/Sponza/glTF/";
			const char* _name = "Sponza.gltf";
			LoadModel(_path, _name);
			m_SceneDirty = true;
			return;
		}

		bool Scene::LoadModel_ALT(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			char filename[128];
			tempModel = new R_Model();
			sprintf(filename, "%s%s", _filepath, _modelName);
			sprintf(tempModel->m_Path , "%s",  _modelName);
			auto data = filesystem::read_glTF(_filepath, _modelName, tempModel);
			if(data == nullptr) return false;
			m_StaticModels[m_CurrentStaticModels] = tempModel;
			m_CurrentStaticModels++;
			return true;
		}

		void Scene::LoadCubemapModel(const char* _filepath, const char* _modelName, glm::vec3 _position, glm::vec3 _scale, char* _customTexture)
		{
			//m_Cubemap->m_gltf = LoadModel(_filepath, _modelName, _position, _scale, _customTexture);
		}

		/// Shadow Pass
		void Scene::ShadowPass(VKBackend* _backend, int _CurrentFrame)
		{
			//glm::mat4 shadowProjMat = glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
			glm::mat4 orthogonalProjMat = glm::ortho<float>(g_DirectionalLight->m_Right, -g_DirectionalLight->m_Right,
																												g_DirectionalLight->m_Up, -g_DirectionalLight->m_Up , 
																												g_DirectionalLight->m_Depth, -g_DirectionalLight->m_Depth);
			glm::mat4 lightViewMat = glm::lookAt( g_DirectionalLight->m_Pos, g_DirectionalLight->m_Pos + g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
			glm::mat4 lightProjMat = orthogonalProjMat * lightViewMat;
			auto renderContext = GetVKContext();
			// Clear Color
			VkClearValue clearValue;
			clearValue.depthStencil = { 1.0f, 0 };
			// Update Uniform buffers
			ShadowUniformBufferObject ubo{};
			ubo.view = lightViewMat;
			ubo.projection = lightProjMat;
			ubo.depthMVP = lightProjMat * lightViewMat;
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
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				DynamicBufferObject dynO{};
				dynO.model = model->m_ModelMatrix;
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				dynO.model = glm::scale(dynO.model, model->m_Scale);
				dynO.model = glm::rotate(dynO.model, model->m_RotGRAD, model->m_RotAngle);
				dynO.modelOpts = glm::vec4(0); // 0: miplevel
				dynO.addOpts = glm::vec4(0); // 0: num current Lights
				dynO.aligned[0] =  glm::vec4(0);
				dynO.aligned[1] =  glm::vec4(0);
				
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
					// Draw Loop
					if (mesh->m_Indices.size() > 0)
					{
						vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
						vkCmdDrawIndexed(_backend->m_CommandBuffer[_CurrentFrame], static_cast<uint32_t>(mesh->m_Indices.size()), 1, 0, 0, 0);
					}
					else
					{
						vkCmdDraw(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_Vertices.size(), 1, 0, 0);
					}
				}
				++count;
				// Flush to make changes visible to the host
				VkMappedMemoryRange mappedMemoryRange{};
				mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedMemoryRange.memory = _backend->m_ShadowDynamicBuffersMemory[_CurrentFrame];
				mappedMemoryRange.size = sizeof(dynO);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &mappedMemoryRange);
			}
			vkCmdEndRenderPass(_backend->m_CommandBuffer[_CurrentFrame]);
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
			PERF_INIT()
			auto renderContext = GetVKContext();
			if(m_GraphicsRender->m_VertShader->GLSLCompile(true) &&
				m_GraphicsRender->m_FragShader->GLSLCompile(true))
			{
				vkDeviceWaitIdle(renderContext.m_LogicDevice);
				vkDestroyPipeline(renderContext.m_LogicDevice, m_GraphicsRender->m_Pipeline, nullptr);
				vkDestroyPipelineLayout(renderContext.m_LogicDevice, m_GraphicsRender->m_PipelineLayout, nullptr);
				m_GraphicsRender->Initialize();
				m_GraphicsRender->CreatePipelineLayoutSetup(&_backend->m_CurrentExtent, &_backend->m_Viewport, &_backend->m_Scissor);
				m_GraphicsRender->CreatePipelineLayout();
				m_GraphicsRender->CreatePipeline(g_context.m_RenderPass->m_Pass);
				m_GraphicsRender->CleanShaderModules();
			}
			PERF_END("RELOAD SHADERS")
		}

		void Scene::DrawScene(VKBackend* _backend, int _CurrentFrame)
		{
			auto imageIdx = _backend->BeginFrame(_CurrentFrame);
			//editor->Loop(this, _backend);
			if(m_SceneDirty)
			{
				m_CurrentStaticModels += m_WaitingModels;
				m_WaitingModels = 0;
				PrepareScene(_backend);
				m_SceneDirty = false;
			}
			auto renderContext = GetVKContext();
			// GeometryPass(_backend, _CurrentFrame);
			ShadowPass(_backend, _CurrentFrame);
			auto dynamicAlignment = sizeof(DynamicBufferObject);
			dynamicAlignment = (dynamicAlignment + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			auto lightDynAlign = sizeof(LightBufferObject);
			lightDynAlign = (lightDynAlign + renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1)
				& ~(renderContext.m_GpuInfo.minUniformBufferOffsetAlignment - 1);
			// Matriz de proyeccion
			glm::mat4 projMat = glm::perspective(glm::radians(m_CameraFOV), m_Width / (float)m_Height, zNear, zFar);
			projMat[1][1] *= -1; // para invertir el eje Y
			glm::mat4 viewMat = glm::lookAt(m_CameraPos, m_CameraPos + m_CameraForward, m_CameraUp);
			/// Render Pass
			// Update Uniform buffers
			UniformBufferObject ubo{};
			ubo.view = viewMat;
			ubo.projection = projMat;
			ubo.cameraPosition = m_CameraPos;
			memcpy(_backend->m_Uniform_SBuffersMapped[_CurrentFrame], &ubo, sizeof(ubo));
			_backend->BeginRenderPass(_CurrentFrame);
			// Drawing Commands
			vkCmdBindPipeline(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsRender->m_Pipeline);
			// REFRESH RENDER MODE FUNCTIONS
			//vkCmdSetViewport(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Viewport);
			//vkCmdSetScissor(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, &_backend->m_Scissor);
			uint32_t count = 0;
			// lights
			m_LightsOs.clear();
			// Ambient Light
			{
				glm::mat4 lightViewMat = glm::lookAt( g_DirectionalLight->m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
				glm::mat4 lightProjMat	= glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
				LightBufferObject temp;
				temp			= {};
				temp.addOpts	= glm::vec4(g_DirectionalLight->m_Pos, 1.0);
				temp.Opts.x		= g_ShadowBias;
				temp.Opts.y		= g_ShadowBias;
				temp.View		= lightViewMat;
				temp.Proj		= lightProjMat;
				temp.Position	= glm::vec4(g_DirectionalLight->m_Pos, 1.0);
				temp.Color		= glm::vec4(g_DirectionalLight->m_Color, 1.0);
				m_LightsOs.push_back(temp);
			}
			//Point Ligts
			for(auto& light : g_Lights)
			{
				glm::mat4 lightViewMat = glm::lookAt( light->m_Pos, g_DirectionalLight->m_Center, g_DirectionalLight->m_UpVector);
				glm::mat4 lightProjMat	= glm::perspective(glm::radians(m_ShadowCameraFOV), g_ShadowAR, zNear, zFar);
				LightBufferObject temp;
				temp			= {};
				temp.addOpts	= glm::vec4(light->m_Pos, 1.0);
				temp.Opts.x		= g_ShadowBias;
				temp.Opts.y		= g_ShadowBias;
				temp.View		= lightViewMat;
				temp.Proj		= lightProjMat;
				temp.Position	= glm::vec4(light->m_Pos, 1.0);
				temp.Color		= glm::vec4(light->m_Color, 1.0);
				m_LightsOs.push_back(temp);
			}
			uint32_t lightDynamicOffset0 = (count + 0) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset1 = (count + 1) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset2 = (count + 2) * static_cast<uint32_t>(lightDynAlign);
			uint32_t lightDynamicOffset3 = (count + 3) * static_cast<uint32_t>(lightDynAlign);
			memcpy((char*)_backend->m_LightsBuffersMapped[_CurrentFrame]  + lightDynamicOffset0, m_LightsOs.data(), m_LightsOs.size() * sizeof(LightBufferObject));
			// Render Pass
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				DynamicBufferObject dynO{};
				dynO.model = model->m_ModelMatrix;
				dynO.model = glm::translate(dynO.model, model->m_Pos);
				dynO.model = glm::scale(dynO.model, model->m_Scale);
				dynO.model = glm::rotate(dynO.model, model->m_RotGRAD, model->m_RotAngle);
				dynO.modelOpts.x = model->m_ProjectShadow; // project shadow
				dynO.modelOpts.y = g_MipLevel;
				dynO.addOpts.x = m_LightsOs.size();
				
				uint32_t dynamicOffset = count * static_cast<uint32_t>(dynamicAlignment);
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_DynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				for (auto& mesh : model->m_Meshes)
				{
					// Update Uniform buffers

					VkBuffer vertesBuffers[] = { mesh->m_VertexBuffer };
					VkDeviceSize offsets[] = { 0 };
					std::vector<uint32_t> dynOffsets = {
						dynamicOffset,
						lightDynamicOffset0,
						lightDynamicOffset1,
						lightDynamicOffset2,
						lightDynamicOffset3,
					};
					vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsRender->m_PipelineLayout, 0, 1,
						&model->m_Materials[mesh->m_Material]->m_DescriptorSet[_CurrentFrame], 5, dynOffsets.data());
					vkCmdBindVertexBuffers(_backend->m_CommandBuffer[_CurrentFrame], 0, 1, vertesBuffers, offsets);
					// Draw Loop
					if ( mesh->m_Indices.size() > 0)
					{
						vkCmdBindIndexBuffer(_backend->m_CommandBuffer[_CurrentFrame], mesh->m_IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
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
				VkMappedMemoryRange lightsMappedMemoryRange{};
				lightsMappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				lightsMappedMemoryRange.memory = _backend->m_LightsBuffersMemory[_CurrentFrame];
				lightsMappedMemoryRange.size = m_LightsOs.size() * sizeof(LightBufferObject);
				vkFlushMappedMemoryRanges(renderContext.m_LogicDevice, 1, &lightsMappedMemoryRange);
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
				dynO.model = glm::translate(dynO.model, g_DirectionalLight->m_Pos);
				dynO.model = glm::scale(dynO.model, glm::vec3(1.f) * g_debugScale);
				dynO.model = glm::rotate<float>(dynO.model, g_Rotation, m_Rotation);

				uint32_t dynamicOffset = debugCount * static_cast<uint32_t>(dynamicAlignment);
				VkBuffer vertesBuffers[] = { model->m_VertexBuffer };
				VkDeviceSize offsets[] = { 0 };
				// OJO aqui hay que sumarle el offset para guardar donde hay que guardar
				memcpy((char*)_backend->m_DbgDynamicBuffersMapped[_CurrentFrame] + dynamicOffset, &dynO, sizeof(dynO));
				vkCmdBindDescriptorSets(_backend->m_CommandBuffer[_CurrentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, m_DbgRender->m_PipelineLayout, 0, 1, 
					&model->m_Material->m_DescriptorSet[_CurrentFrame], 1, &dynamicOffset);
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
			if(g_DrawCubemap)
				DrawCubemapScene(_backend, _CurrentFrame, projMat, viewMat, static_cast<uint32_t>(dynamicAlignment));
			_backend->EndRenderPass(_CurrentFrame);
			_backend->SubmitAndPresent(_CurrentFrame, &imageIdx);
		}

		void Scene::PrepareCubemapScene(VKBackend* _backend)
		{
			PERF_INIT()
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
			PERF_END("PREPARE CUBEMAP")
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
			/// 1 - Actualizar los DynamicDescriptorBuffers
			//_backend->GenerateBuffers();
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
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
				for (auto& mesh : model->m_Meshes)
				{
					if(mesh->m_VertexBuffer && mesh->m_IndexBuffer)
						continue;
					PERF_INIT()
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
						_backend->m_UniformBuffers, _backend->m_DynamicBuffers, _backend->m_LightsBuffers);
					PERF_END("PREPARE DRAW SCENE")
				}
			}

			/// 8 - (OPCIONAL)Reordenar modelos
			// Vamos a pre-ordenar los modelos para pintarlos segun el material.
			// BUBBLESORT de primeras, luego ya veremos, al ser tiempo pre-frameloop, no deberia importar.
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
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
		void Scene::Init(VKBackend* _backend)
		{
			m_Cubemap = new R_Cubemap("resources/Textures/cubemaps/cubemaps_skybox_3.png");
			g_DirectionalLight = new Directional();
			PrepareDebugScene(_backend);
		}
		void Scene::PrepareDebugScene(VKBackend* _backend)
		{
			auto renderContext = GetVKContext();
            _backend->m_CurrentDebugModelsToDraw = m_DbgModels.size();
			_backend->GenerateDBGBuffers();
			for (auto& dbgModel : m_DbgModels)
			{
				dbgModel->m_Material->m_Texture->LoadTexture();
				/// 2 - Crear descriptor pool de materiales(CreateDescPool)`
				dbgModel->m_Material->CreateDescriptorPool(renderContext.m_LogicDevice);

				/// 3 - Crear Descriptor set de material(createMeshDescSet)
				dbgModel->m_Material->CreateMeshDescriptorSet(renderContext.m_LogicDevice, m_DbgRender->m_DescSetLayout);
				/// 4 - Crear y transicionar texturas(CreateAndTransImage)
				dbgModel->m_Material->m_Texture->CreateAndTransitionImageNoMipMaps(_backend->m_CommandPool);
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

				dbgModel->m_Material->UpdateDescriptorSet(renderContext.m_LogicDevice, _backend->m_DbgUniformBuffers, _backend->m_DbgDynamicBuffers);

			}
		}

		void Scene::Cleanup(VkDevice _LogicDevice)
		{
			printf("Scene Cleanup\n");
			vkDeviceWaitIdle(_LogicDevice);
			for (int i = 0; i < m_CurrentStaticModels; i++)
			{
				R_Model* model = m_StaticModels[i];
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
