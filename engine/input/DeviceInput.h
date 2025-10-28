#pragma once

#include "../Camera.h"
#ifndef USE_GLFW
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "Xinput9_1_0.lib")

inline XINPUT_STATE devices_state[XUSER_MAX_COUNT];
void init_input_devices();
void process_input();
void reset_devices();
#endif

#ifdef USE_GLFW
#include <GLFW/glfw3.h>
void MouseCallback(GLFWwindow* _window, double _xPos, double _yPos);
void MouseBPressCallback(GLFWwindow* _window, int _button, int _action, int _mods);
void KeyboardInputCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
#endif