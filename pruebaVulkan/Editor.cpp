#include "Editor.h"
#include "VKBackend.h"

#include <string>

namespace VKR
{
	std::string g_ConsoleMSG;
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
		ImGui_ImplVulkan_Init(&init_info, renderContext.m_RenderPass);
	}

	Editor::~Editor()
	{
		auto renderContext = VKR::render::GetVKContext();
		vkDestroyDescriptorPool(renderContext.m_LogicDevice, m_UIDescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void Editor::Loop()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#if 1
		ImGui::Begin("Tools");
		{
			float tempLightPos[3];
			ImGui::LabelText("Light Pos", "Light Pos(%.2f, %.2f, %.2f)", render::m_LightPos.x, render::m_LightPos.y, render::m_LightPos.z);
			tempLightPos[0] = render::m_LightPos.x;
			tempLightPos[1] = render::m_LightPos.y;
			tempLightPos[2] = render::m_LightPos.z;
			ImGui::SliderFloat3("Light Position", tempLightPos, -10.0f, 10.0f);
			render::m_LightPos.x = tempLightPos[0];
			render::m_LightPos.y = std::max(0.0f, tempLightPos[1]);
			render::m_LightPos.z = tempLightPos[2];
			// Camera
			ImGui::SliderFloat("cam Speed", &render::m_CameraSpeed, 0.1f, 100.f);
			ImGui::SliderFloat("FOV", &render::m_CameraFOV, 40.f, 100.f);
			ImGui::Checkbox("Indexed Draw", &render::m_IndexedRender);
			ImGui::Checkbox("Debug Draw", &render::m_DebugRendering);
			ImGui::LabelText("Cam Pos", "Cam Pos(%.2f, %.2f, %.2f)", render::m_CameraPos.x, render::m_CameraPos.y, render::m_CameraPos.z);
			ImGui::LabelText("Cam Pitch", "Cam Pitch(%.2f)", render::m_CameraPitch);
			ImGui::LabelText("Cam Yaw", "Cam Yaw(%.2f)", render::m_CameraYaw);

			if (ImGui::Button("Center Light"))
			{
				render::m_LightPos = glm::vec3(0.0f);
			}
			ImGui::End();
		}
		ImGui::Begin("Lighting");
		{
			float color[3];
			color[0] = render::m_LightColor.x;
			color[1] = render::m_LightColor.y;
			color[2] = render::m_LightColor.z;
			ImGui::ColorEdit3("Color", color);
			render::m_LightColor.x = color[0];
			render::m_LightColor.y = color[1];
			render::m_LightColor.z = color[2];
			float forward[3];
			forward[0] = render::m_LightForward.x;
			forward[1] = render::m_LightForward.y;
			forward[2] = render::m_LightForward.z;
			ImGui::SliderFloat3("Cam orientation", forward, 0.0f, 1.0f);
			render::m_LightForward.x = forward[0];
			render::m_LightForward.y = forward[1];
			render::m_LightForward.z = forward[2];
			/*if (m_ShadowVisualizer == nullptr)
				m_ShadowVisualizer = ImGui_ImplVulkan_AddTexture(m_ShadowImgSamp, m_ShadowImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			ImGui::Image(m_ShadowVisualizer, ImVec2{ viewportPanelSize.x, viewportPanelSize.y });*/
			ImGui::End();
		}
		ImGui::Begin("World");
		{
#if 0
			for (auto& model : m_StaticModels)
			{
				float position[3] = { model->m_Pos.x , model->m_Pos.y, model->m_Pos.z };
				ImGui::LabelText("%s ", model->m_Path);
				ImGui::DragFloat3("Position", position, 0.01f, -100.f, 100.f);
				model->m_Pos.x = position[0];
				model->m_Pos.y = position[1];
				model->m_Pos.z = position[2];
			}
#endif
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
