#pragma once

#include "../Camera.h"
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")

XINPUT_STATE devices_state[XUSER_MAX_COUNT];

inline void 
init_input_devices(HINSTANCE _hInstance)
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
inline void
reset_devices()
{
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    vibration.wLeftMotorSpeed = 0; // use any value between 0-65535 here
    vibration.wRightMotorSpeed = 0; // use any value between 0-65535 here
    XInputSetState(0, &vibration);
}

inline void 
process_input()
{
    auto result = XInputGetState(0, &devices_state[0]);
    if (result != ERROR_SUCCESS)
    {
        fprintf(stderr, "Gamepad disconnected %d\n", 0);
    }
    if (devices_state[0].Gamepad.sThumbLY > 0)
    {
        // forward
        camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraForward;
    }
    if (devices_state[0].Gamepad.sThumbLY < 0)
    {
        camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraForward;
    }
    if (devices_state[0].Gamepad.sThumbLX > 0)
    {
        camera.g_CameraPos -= camera.m_CameraSpeed * glm::normalize(glm::cross(
            camera.g_CameraUp, camera.g_CameraForward));
    }
    if (devices_state[0].Gamepad.sThumbLX < 0)
    {
        camera.g_CameraPos += camera.m_CameraSpeed * glm::normalize(glm::cross(
            camera.g_CameraUp, camera.g_CameraForward));
    }
    if (devices_state[0].Gamepad.sThumbRY > 0)
    {
        camera.g_CameraPos += camera.m_CameraSpeed * camera.g_CameraUp;
    }
    if (devices_state[0].Gamepad.sThumbRY < 0)
    {
        camera.g_CameraPos -= camera.m_CameraSpeed * camera.g_CameraUp;
    }
    if (devices_state[0].Gamepad.wButtons == XINPUT_GAMEPAD_A)
    {
        camera.g_CameraPos = camera.g_CameraDefPos;
    }
    if (devices_state[0].Gamepad.wButtons == XINPUT_GAMEPAD_START)
    {
        ;
    }
    if (devices_state[0].Gamepad.wButtons == XINPUT_GAMEPAD_BACK)
    {
        ;
    }
}