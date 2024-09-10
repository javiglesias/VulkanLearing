#include "Editor.h"
#include "EditorModels.h"
#include "../video/VKBackend.h"
#include "../core/VKRScene.h"
#include <cstdio>

#ifdef _WINDOWS
#include "../../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../../dependencies/imgui/backends/imgui_impl_vulkan.h"
#else
#include "../../../dependencies/imgui/imgui.h"
#include "../../../dependencies/imgui/misc/single_file/imgui_single_file.h"
#include "../../../dependencies/imgui/backends/imgui_impl_glfw.h"
#include "../../../dependencies/imgui/backends/imgui_impl_vulkan.h"
#include <dirent.h>
#endif

namespace VKR
{
	namespace render
	{
		extern std::string g_ConsoleMSG;
		Editor::Editor(GLFWwindow* _Window, VkInstance _Instance, uint32_t _MinImageCount, uint32_t _ImageCount)
		{
			auto renderContext = VKR::render::GetVKContext();
			IMGUI_CHECKVERSION();
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
			// // list all the directories for the model loading
			// const char* path = "./resources/models";
			// DIR* directory = opendir(path);
			// dirent* entry = readdir(directory);
			// while(entry != NULL)
			// {
			// 	m_Models.push_back(entry->d_name);
			// 	entry = readdir(directory);
			// }
			// closedir(directory);
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

				if (ImGui::Button("Load cgltf"))
				{
					if(_mainScene->LoadModel_ALT("resources/models/Avocado/glTF/", "Avocado.gltf", glm::vec3(0.f, 0.f, 0.f)))
					{
						VKR::render::m_CreateTestModel = false;
						_mainScene->PrepareScene(_backend);
					} else {
					fprintf(stderr, "ERROR LOADING MODEL WITH cgltf.h");
					}
				}
				if (ImGui::Button("Reload shaders"))
				{
					_mainScene->ReloadShaders(_backend);
				}

				 static ImGuiComboFlags flags = 0;
				 static int item_current_idx = 0;
				  const char* combo_preview_value =ModelList[0];
				if (ImGui::BeginCombo("Load model", combo_preview_value, flags))
				{
					for (int n = 1; n < MODELS_SIZE; n++)
					{
						const bool is_selected = (item_current_idx == n);
						if (ImGui::Selectable(ModelList[n], is_selected))
							item_current_idx = n;

						// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}
				ImGui::SameLine();
				if (ImGui::Button("Load"))
				{
					char _filepath[128];
					sprintf(_filepath, "resources/models/%s/glTF/", ModelList[item_current_idx]);
					char modelName[32];
					sprintf(modelName, "%s.gltf", ModelList[item_current_idx]);
					_mainScene->LoadModel_ALT(_filepath, modelName, glm::vec3(0.f, 0.f, 0.f));
					_mainScene->PrepareScene(_backend);
				}

				ImGui::DragFloat("zFar", &zFar);
				ImGui::DragFloat("zNear", &zNear);
				ImGui::DragFloat("Cubemap distance", &g_cubemapDistance);
				ImGui::Checkbox("Draw Cubemap", &g_DrawCubemap);
				ImGui::SliderFloat("Mip level", &g_MipLevel, 0.f, 12.f, "%1.f");
				ImGui::DragFloat("Debug scale", &g_debugScale);
				ImGui::End();
			}
			ImGui::Begin("Debug");
			{
				/*if (_backend->m_ShadowVisualizer == nullptr)
					_backend->m_ShadowVisualizer = ImGui_ImplVulkan_AddTexture(_backend->m_ShadowImgSamp, _backend->m_ShadowImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
				ImGui::Image(_backend->m_ShadowVisualizer, ImVec2{ viewportPanelSize.x, viewportPanelSize.y });*/
				ImGui::LabelText("Elapsed time:", "%.2f", g_ElapsedTime);
				ImGui::LabelText("Frame time:", "%.2f", g_FrameTime[g_CurrentFrameTime]);
				ImGui::LabelText("Frame n:", "%lld", g_CurrentFrame);
				const float *cycles = g_FrameTime;
				ImGui::PlotLines("Frame time", cycles, g_FrameGranularity);

				ImGui::End();
			}
			ImGui::Begin("World");
			{
				for (int i = 0; i < m_CurrentStaticModels; i++)
				{
					R_Model* model = m_StaticModels[i];
					ImGui::Selectable(model->m_Path, &model->m_Editable);
					if (model->m_Editable)
					{
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
						
						float scale[3];
						scale[0] = model->m_Scale.x;
						scale[1] = model->m_Scale.y;
						scale[2] = model->m_Scale.z;
						ImGui::InputFloat3("Scale", scale);
						model->m_Scale.x = scale[0];
						model->m_Scale.y = scale[1];
						model->m_Scale.z = scale[2];
					}
				}
				/*if (ImGui::Button("Delete"))
				{
					m_StaticModels.pop_back();
				}*/
				ImGui::End();
			}
			ImGui::Begin("Lights");
			{
				if (ImGui::Button("Create Point Light"))
				{
					// TO-DO: Max 4 Lights rait nao.
					if(g_Lights.size() < 4)
					{
						g_Lights.push_back(new Light());
						_mainScene->PrepareDebugScene(_backend);
					}
				}

				if (ImGui::Button("Create Spot Light"))
				{

				}
				// Directional Light opts
				if (ImGui::TreeNode("Directional Light settings"))
				{
					float position[3];
					position[0] = g_DirectionalLight->m_Pos.x;
					position[1] = g_DirectionalLight->m_Pos.y;
					position[2] = g_DirectionalLight->m_Pos.z;
					ImGui::DragFloat3("Pos", position, 0.1f);
					g_DirectionalLight->m_Pos.x = position[0];
					g_DirectionalLight->m_Pos.y = position[1];
					g_DirectionalLight->m_Pos.z = position[2];
					ImGui::DragFloat("Right size", &g_DirectionalLight->m_Right);
					ImGui::DragFloat("Up size", &g_DirectionalLight->m_Up);
					ImGui::DragFloat("Depth size", &g_DirectionalLight->m_Depth);
					float center[3];
					center[0] = g_DirectionalLight->m_Center.x;
					center[1] = g_DirectionalLight->m_Center.y;
					center[2] = g_DirectionalLight->m_Center.z;
					ImGui::DragFloat3("Look At", center, 0.1f);
					g_DirectionalLight->m_Center.x = center[0];
					g_DirectionalLight->m_Center.y = center[1];
					g_DirectionalLight->m_Center.z = center[2];
					ImGui::TreePop();
				}
				for (auto& light : g_Lights)
				{
					ImGui::Selectable("light", &light->m_Editable);
					if(light->m_Editable)
					{
						float center[3];
						center[0] = light->m_Pos.x;
						center[1] = light->m_Pos.y;
						center[2] = light->m_Pos.z;
						ImGui::DragFloat3("Pos", center, 0.1f);
						light->m_Pos.x = center[0];
						light->m_Pos.y = center[1];
						light->m_Pos.z = center[2];
					}
				}
				if (ImGui::Button("Delete"))
				{
					g_Lights.pop_back();
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