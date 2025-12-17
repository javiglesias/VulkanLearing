#ifndef _C_UTILS
#define _C_UTILS


#ifdef _WIN32
#include <Windows.h>
#else
#include <cstring>
#endif
#include <string>
#include <cstdio>
#include <cmath>

inline
static void VK_ASSERT(bool _check)
{
	if (_check)
	{
		exit(-69);
	}
}

enum CONSOLE_COLOR
{
	BLUE = 9,
	GREEN = 10,
	RED = 12,
	PURPLE = 13,
	YELLOW = 14,
	NORMAL = 15
};
#ifdef WIN32
inline HANDLE  hConsole;
#endif
inline
void ChangeColorConsole(CONSOLE_COLOR _color)
{
#ifdef WIN32
	if (!hConsole)
		hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, _color);
#endif
}

#endif
