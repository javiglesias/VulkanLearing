#include "Editor.h"
#include "VKRScene.h"
#include "jobs.h"
#include "savedata.h"
#include <string>

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		Editor::Editor(GLFWwindow* _Window, VkInstance _Instance, uint32_t _MinImageCount, uint32_t _ImageCount)
		{
			auto renderContext = VKR::render::GetVKContext();
			// IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGui_ImplGlfw_InitForVulkan(_Window, true);
			// UI descriptor Pool
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};
			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			vkCreateDescriptorPool(renderContext.m_LogicDevice, &pool_info, nullptr, &m_UIDescriptorPool);

			// IMGUI
			ImGui_ImplVulkan_InitInfo init_info = {};
			init_info.Instance = _Instance;
			init_info.PhysicalDevice = renderContext.m_GpuInfo.m_Device;
			init_info.Device = renderContext.m_LogicDevice;
			init_info.QueueFamily = renderContext.m_GraphicsQueueFamilyIndex;
			init_info.Queue = renderContext.m_GraphicsQueue;
			init_info.PipelineCache = VK_NULL_HANDLE;
			init_info.DescriptorPool = m_UIDescriptorPool;
			init_info.Allocator = nullptr;
			init_info.MinImageCount = _MinImageCount;
			init_info.ImageCount = _ImageCount;
			init_info.CheckVkResultFn = nullptr;
			ImGui_ImplVulkan_Init(&init_info, renderContext.m_RenderPass->m_Pass);
		}
		void Editor::Cleanup()
		{
			printf("Editor Cleanup\n");
			auto renderContext = VKR::render::GetVKContext();
			vkDeviceWaitIdle(renderContext.m_LogicDevice);
			ImGui_ImplVulkan_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
			vkDestroyDescriptorPool(renderContext.m_LogicDevice, m_UIDescriptorPool, nullptr);
		}

		void Editor::Shutdown()
		{
			printf("Editor shutdown\n");
		}
		Editor::~Editor()
		{
		}


		void Editor::Loop(Scene* _mainScene ,VKBackend* _backend)
		{
			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();
#if 1
			ImGui::Begin("Tools");
			{
				// Camera
				ImGui::SliderFloat("cam Speed", &render::m_CameraSpeed, 0.1f, 100.f);
				ImGui::SliderFloat("FOV", &render::m_CameraFOV, 40.f, 100.f);
				ImGui::LabelText("Cam Pos", "Cam Pos(%.2f, %.2f, %.2f)", render::m_CameraPos.x, render::m_CameraPos.y, render::m_CameraPos.z);
				ImGui::DragFloat("Rotation", &render::g_Rotation, 1.f, 0.f, 360.f);
				float rotation[3];
				rotation[0] = render::m_Rotation.x;
				rotation[1] = render::m_Rotation.y;
				rotation[2] = render::m_Rotation.z;
				ImGui::InputFloat3("Rotation", rotation);
				render::m_Rotation.x = rotation[0];
				render::m_Rotation.y = rotation[1];
				render::m_Rotation.z = rotation[2];
				if (ImGui::Button("Load demo"))
				{
					_mainScene->LoadStaticModel("resources/models/Plane/glTF/", "Plane.gltf", glm::vec3(0.f, 1.f, 0.f));
					_mainScene->LoadStaticModel("resources/models/scene/glTF/", "scene.gltf", glm::vec3(0.f, 2.f, 0.f));
					_mainScene->LoadStaticModel("resources/models/Sponza/glTF/", "Sponza.gltf", glm::vec3(0.f, 2.f, 0.f));
					VKR::render::m_CreateTestModel = false;
					_mainScene->PrepareScene(_backend);
				}
				if (ImGui::Button("Reload shaders"))
				{
					_mainScene->ReloadShaders(_backend);
				}
				if (ImGui::Button("Create Terrain debug"))
				{
					_mainScene->CreateDebugModel(TERRAIN);
					_mainScene->PrepareDebugScene(_backend);
				}
				ImGui::End();
			}
			ImGui::Begin("Lighting");
			{
				ImGui::LabelText("Light Pos", "Light Pos(%.2f, %.2f, %.2f)", render::m_LightPos.x, render::m_LightPos.y, render::m_LightPos.z);

				float color[3];
				color[0] = render::m_LightColor.x;
				color[1] = render::m_LightColor.y;
				color[2] = render::m_LightColor.z;
				ImGui::ColorEdit3("Color", color);
				render::m_LightColor.x = color[0];
				render::m_LightColor.y = color[1];
				render::m_LightColor.z = color[2];
				ImGui::DragFloat("zFar", &render::zFar);
				ImGui::DragFloat("zNear", &render::zNear);
				ImGui::DragFloat("Debug Scale", &render::g_debugScale);
				ImGui::DragFloat("Cubemap distance", &render::g_cubemapDistance);
				ImGui::DragFloat("Shadow Fov", &render::m_ShadowCameraFOV);
				ImGui::DragFloat("Shadow aspect ratio", &render::g_ShadowAR);
				ImGui::SliderFloat("Shadow bias", &render::g_ShadowBias, 0.0025f, 1.f, "%.5f");
				ImGui::SliderFloat("Mip level", &render::g_MipLevel, 0.f, 12.f, "%1.f");

				ImGui::LabelText("ProjMat", "Proj Matrix");
				ImGui::InputFloat("Light Right ortho", &render::g_LightRight);
				ImGui::InputFloat("Light Up ortho", &render::g_LightUp);
				ImGui::InputFloat("Light Depth ortho", &render::g_LightDepth);

				ImGui::LabelText("ViewMat", "View Matrix");
				float tempLightPos[3];
				tempLightPos[0] = render::m_LightPos.x;
				tempLightPos[1] = render::m_LightPos.y;
				tempLightPos[2] = render::m_LightPos.z;
				ImGui::DragFloat3("Light Position", tempLightPos);
				render::m_LightPos.x = tempLightPos[0];
				render::m_LightPos.y = std::max(0.0f, tempLightPos[1]);
				render::m_LightPos.z = tempLightPos[2];
				float up[3];
				up[0] = render::m_LightUp.x;
				up[1] = render::m_LightUp.y;
				up[2] = render::m_LightUp.z;
				ImGui::DragFloat3("Light Up", up);
				render::m_LightUp.x = up[0];
				render::m_LightUp.y = up[1];
				render::m_LightUp.z = up[2];
				float center[3];
				center[0] = render::m_LightCenter.x;
				center[1] = render::m_LightCenter.y;
				center[2] = render::m_LightCenter.z;
				ImGui::DragFloat3("Light Center", center);
				render::m_LightCenter.x = center[0];
				render::m_LightCenter.y = center[1];
				render::m_LightCenter.z = center[2];

				if (ImGui::Button("Create light sphere"))
				{
					_mainScene->CreateDebugModel(SPHERE);
					_mainScene->PrepareDebugScene(_backend);
				}
				if (ImGui::Button("Create light quad"))
				{
					_mainScene->CreateDebugModel(QUAD);
					_mainScene->PrepareDebugScene(_backend);
				}
				if (_backend->m_ShadowVisualizer == nullptr)
					_backend->m_ShadowVisualizer = ImGui_ImplVulkan_AddTexture(_backend->m_ShadowImgSamp, _backend->m_ShadowImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				ImGui::Image(_backend->m_ShadowVisualizer, ImVec2{ viewportPanelSize.x, viewportPanelSize.y });
				ImGui::End();
			}
			ImGui::Begin("World");
			{
	
				for (auto& model : _mainScene->m_StaticModels)
				{
					ImGui::Selectable(model->m_Path, &model->m_Editable);
					if (model->m_Editable)
					{
						ImGui::LabelText("", "%s", model->m_Path);
						float center[3];
						center[0] = model->m_Pos.x;
						center[1] = model->m_Pos.y;
						center[2] = model->m_Pos.z;
						ImGui::DragFloat3("Pos", center, 0.1f);
						model->m_Pos.x = center[0];
						model->m_Pos.y = center[1];
						model->m_Pos.z = center[2];
						ImGui::DragFloat("P.Shadow", &model->m_ProjectShadow);

						float rotation[3];
						rotation[0] = model->m_RotAngle.x;
						rotation[1] = model->m_RotAngle.y;
						rotation[2] = model->m_RotAngle.z;
						ImGui::InputFloat3("Rot Angle", rotation);
						model->m_RotAngle.x = rotation[0];
						model->m_RotAngle.y = rotation[1];
						model->m_RotAngle.z = rotation[2];
						ImGui::DragFloat("R.GRAD", &model->m_RotGRAD);

					}
				}
				ImGui::End();
			}

			ImGui::Begin("DEBUG PANEL");
			{
				ImGui::Text("%s", g_ConsoleMSG.c_str());
				bool isOpen = true;
				//ImGui::ShowDemoWindow(&isOpen);
				ImGui::End();
			}
#endif
			ImGui::EndFrame();
		}

		void Editor::Draw(VkCommandBuffer _commandBuffer)
		{
			// Render ImGui
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// UI Render
			if (draw_data)
				ImGui_ImplVulkan_RenderDrawData(draw_data, _commandBuffer);

		}
	}
}
