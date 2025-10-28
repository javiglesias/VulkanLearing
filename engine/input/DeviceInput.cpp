#include "DeviceInput.h"
#include <glm/glm.hpp>

#ifndef USE_GLFW
void init_input_devices()
{
	memset(&devices_state, 0, XUSER_MAX_COUNT * sizeof(XINPUT_STATE));
	for (auto i = 0; i < XUSER_MAX_COUNT; i++)
	{
		auto result = XInputGetState(i, &devices_state[i]);
		if (result == ERROR_SUCCESS)
		{
			printf("Gamepad connected %d\n", i);
		}
	}
}
void
reset_devices()
{
	XINPUT_VIBRATION vibration;
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
	vibration.wLeftMotorSpeed = 0; // use any value between 0-65535 here
	vibration.wRightMotorSpeed = 0; // use any value between 0-65535 here
	XInputSetState(0, &vibration);
}

void
process_input()
{
	for (auto i = 0; i < XUSER_MAX_COUNT; i++)
	{
		auto result = XInputGetState(i, &devices_state[i]);
		if (result != ERROR_SUCCESS)
		{
			fprintf(stderr, "Gamepad disconnected %d\n", 0);
		}
		if (devices_state[i].Gamepad.sThumbLY > 0)
		{
			// forward
			camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraForward;
		}
		if (devices_state[i].Gamepad.sThumbLY < 0)
		{
			camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraForward;
		}
		if (devices_state[i].Gamepad.sThumbLX > 0)
		{
			camera.g_CameraPos -= camera.m_CameraSpeed * glm::normalize(glm::cross(
				camera.g_CameraUp, camera.g_CameraForward));
		}
		if (devices_state[i].Gamepad.sThumbLX < 0)
		{
			camera.g_CameraPos += camera.m_CameraSpeed * glm::normalize(glm::cross(
				camera.g_CameraUp, camera.g_CameraForward));
		}
		if (devices_state[i].Gamepad.sThumbRY > 0)
		{
			camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraUp;
		}
		if (devices_state[i].Gamepad.sThumbRY < 0)
		{
			camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraUp;
		}
		if (devices_state[i].Gamepad.wButtons == XINPUT_GAMEPAD_A)
		{
			camera.g_CameraPos = camera.g_CameraDefPos;
		}
		if (devices_state[i].Gamepad.wButtons == XINPUT_GAMEPAD_START)
		{
			//VKR::render::m_CloseEngine = true;
		}
		if (devices_state[i].Gamepad.wButtons == XINPUT_GAMEPAD_BACK)
		{
			;
		}
	}
}
#endif
// INPUT CALLBACKS
#ifdef USE_GLFW

void 
MouseCallback(GLFWwindow* _window, double _xPos, double _yPos)
{
#if 0
	double x_offset = (_xPos - m_LastXPosition);
	double y_offset = (m_LastYPosition - _yPos);
	float senseo = 0.01f;
	if (m_MouseCaptured)
	{
		m_LastXPosition = _xPos;
		m_LastYPosition = _yPos;
		x_offset *= senseo;
		y_offset *= senseo;
		m_CameraYaw += x_offset;
		m_CameraPitch += y_offset;
		// CONSTRAINTS
		if (m_CameraPitch > 89.f)  m_CameraPitch = 89.0f;
		if (m_CameraPitch < -89.f) m_CameraPitch = -89.0f;
		glm::vec3 camera_direction;
		camera_direction.x = static_cast<float>(cos(glm::radians(m_CameraYaw) * cos(glm::radians(m_CameraPitch))));
		camera_direction.y = static_cast<float>(sin(glm::radians(m_CameraPitch)));
		camera_direction.z = static_cast<float>(sin(glm::radians(m_CameraYaw)) * cos(glm::radians(m_CameraPitch)));
		camera.g_CameraForward = glm::normalize(camera_direction);
		m_LastXPosition = _xPos;
		m_LastYPosition = _yPos;
	}
#endif
}

void 
MouseBPressCallback(GLFWwindow* _window, int _button, int _action, int _mods)
{
}

void 
KeyboardInputCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	auto state = glfwGetKey(_window, _key);
	if (_key == GLFW_KEY_W && state)
	{
		camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraForward;
	}
	if (_key == GLFW_KEY_A && state)
	{
		camera.g_CameraPos += camera.m_CameraSpeed * glm::normalize(glm::cross(
			camera.g_CameraUp, camera.g_CameraForward));
	}
	if (_key == GLFW_KEY_S && state)
	{
		camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraForward;
	}
	if (_key == GLFW_KEY_D && state)
	{
		camera.g_CameraPos -= camera.m_CameraSpeed * glm::normalize(glm::cross(
			camera.g_CameraUp, camera.g_CameraForward));
	}

	if (_key == GLFW_KEY_R && _action == GLFW_PRESS)
	{
		camera.g_CameraPos = camera.g_CameraDefPos;
	}

	if (_key == GLFW_KEY_TAB && _action == GLFW_PRESS)
	{

	}
	/*
	 * Elemental rotations
	 */
	float degrees = 90.f;
	glm::mat3 X_axis = glm::mat3{
		1, 0, 0,
		0, cos(degrees), -sin(degrees),
		0, sin(degrees), cos(degrees)
	};
	glm::mat3 Y_axis = glm::mat3{
		cos(degrees), 0, sin(degrees),
		0, 1, 0,
		-sin(degrees),0, cos(degrees)
	};
	glm::mat3 Z_axis = glm::mat3{
		cos(degrees), -sin(degrees), 0,
		sin(degrees), cos(degrees), 0,
		0, 0, 1
	};
	if (_key == GLFW_KEY_E && _action == GLFW_PRESS) // up
	{
		camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraUp;
	}
	if (_key == GLFW_KEY_Q && _action == GLFW_PRESS) // down
	{
		camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraUp;
	}

	if (_key == GLFW_KEY_X && _action == GLFW_PRESS) // rotate Right
	{
		camera.g_CameraPos = camera.g_CameraPos * Y_axis;
	}

	if (_key == GLFW_KEY_Z && _action == GLFW_PRESS) // rotate Left
	{
		camera.g_CameraPos = camera.g_CameraPos * -Z_axis;
	}


	//if (_key == GLFW_KEY_C && _action == GLFW_PRESS) // up
	//{
	//	RM::_AddRequest(ASSIMP_MODEL,"resources/models/Sponza/glTF/", "Sponza.gltf");
	//}
	//if (_key == GLFW_KEY_V && _action == GLFW_PRESS) // up
	//{
	//	//RM::_AddRequest(STATIC_MODEL,"resources/models/StainedGlassLamp/glTF/", "StainedGlassLamp.gltf");
	//	auto tempModel = new render::R_Model();
	//	auto data = filesystem::read_glTF("resources/models/StainedGlassLamp/glTF/", "StainedGlassLamp.gltf", tempModel);
	//	render::m_SceneDirty = true;
	//}

	//if (_key == GLFW_KEY_P && _action == GLFW_PRESS) // up
	//{
	//	RM::_AddRequest(STATIC_MODEL, "resources/models/Plane/glTF/", "Plane.gltf");
	//}
}
#endif