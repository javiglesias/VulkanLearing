#include "Editor.h"

#include <string>

namespace VKR
{
	extern std::string g_ConsoleMSG;
	Editor::Editor(GLFWwindow* _Window)
	{
		// IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(_Window, true);
	}

	Editor::~Editor()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}


	void Editor::Loop()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#if 0
		ImGui::Begin("Tools");
		{
			float tempLightPos[3];
			ImGui::LabelText("Light Pos", "Light Pos(%.2f, %.2f, %.2f)", m_LightPos.x, m_LightPos.y, m_LightPos.z);
			tempLightPos[0] = m_LightPos.x;
			tempLightPos[1] = m_LightPos.y;
			tempLightPos[2] = m_LightPos.z;
			ImGui::SliderFloat3("Light Position", tempLightPos, -10.0f, 10.0f);
			m_LightPos.x = tempLightPos[0];
			m_LightPos.y = std::max(0.0f, tempLightPos[1]);
			m_LightPos.z = tempLightPos[2];
			// Camera
			ImGui::SliderFloat("cam Speed", &m_CameraSpeed, 0.1f, 100.f);
			ImGui::SliderFloat("FOV", &m_CameraFOV, 40.f, 100.f);
			ImGui::Checkbox("Indexed Draw", &m_IndexedRender);
			ImGui::Checkbox("Debug Draw", &m_DebugRendering);
			ImGui::LabelText("Cam Pos", "Cam Pos(%.2f, %.2f, %.2f)", m_CameraPos.x, m_CameraPos.y, m_CameraPos.z);
			ImGui::LabelText("Cam Pitch", "Cam Pitch(%.2f)", m_CameraPitch);
			ImGui::LabelText("Cam Yaw", "Cam Yaw(%.2f)", m_CameraYaw);

			if (ImGui::Button("Center Light"))
			{
				m_LightPos = glm::vec3(0.0f);
			}
			ImGui::End();
		}
		ImGui::Begin("Lighting");
		{
			float color[3];
			color[0] = m_LightColor.x;
			color[1] = m_LightColor.y;
			color[2] = m_LightColor.z;
			ImGui::ColorEdit3("Color", color);
			m_LightColor.x = color[0];
			m_LightColor.y = color[1];
			m_LightColor.z = color[2];
			float forward[3];
			forward[0] = m_LightForward.x;
			forward[1] = m_LightForward.y;
			forward[2] = m_LightForward.z;
			ImGui::SliderFloat3("Cam orientation", forward, 0.0f, 1.0f);
			m_LightForward.x = forward[0];
			m_LightForward.y = forward[1];
			m_LightForward.z = forward[2];
			if (m_ShadowVisualizer == nullptr)
				m_ShadowVisualizer = ImGui_ImplVulkan_AddTexture(m_ShadowImgSamp, m_ShadowImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
			ImGui::Image(m_ShadowVisualizer, ImVec2{ viewportPanelSize.x, viewportPanelSize.y });
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
