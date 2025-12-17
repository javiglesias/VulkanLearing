#include "core/VKRScene.h"
#include "video/VKRUtils.h"
#include "video/VKDevice.h"
#include "memory/mem_alloc.h"
#include "perfmon/Custom.h"
#include "filesystem/ResourceManager.h"
//std::thread* RMThread;
void _init_resource_manager()
{
	//VKR::RM::_Init();
}
#ifndef USE_GLFW
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	char _argv[128];
	wcstombs(_argv, pCmdLine, 128);
	AttachConsole(GetCurrentProcessId());
	freopen("console.log", "w", stdout);
	freopen("console.log", "w", stderr);
#else
int main(int _argc, char** _args)
{
#endif
	init_arena(&generic_arena, 20000000);
	auto backend = VKR::render::GetVKBackend();
	auto mainScene = VKR::render::GetVKMainScene();
	int currentLocalFrame = 0;
	double currentFrame = 0.f;
	double nextFrame = 0.f;
	double deltaTime = 0.f;
	auto p = NEW(float);
	//RMThread = new std::thread(_init_resource_manager);
	backend.Init(
#ifndef USE_GLFW
		hInstance
#endif
	);
	char sceneModel[64];
#ifndef USE_GLFW
	if (_argc > 1)
	{
		char pBuf[256];
		size_t len = sizeof(pBuf);
		int bytes = GetModuleFileName(NULL, pBuf, len);
		fprintf(stdout, "Initializing Scene with model -> %s\n", _args[1]);
		sprintf(sceneModel, "%s\0", _args[1]);
	}
#endif

	mainScene.Init(&backend, sceneModel);
	auto renderContext = VKR::render::GetVKContext();
	currentFrame = backend.GetTime();
	 _initlializePerfmon(&backend);
	 bool not_close = true;
	while (not_close = !backend.BackendShouldClose())
	{
		deltaTime = nextFrame - currentFrame;
		VKR::render::g_ElapsedTime += backend.GetTime();
		currentFrame = nextFrame;
		VKR::render::g_FrameTime = deltaTime;
		mainScene.DrawScene(&backend, currentLocalFrame);
		currentLocalFrame = (currentLocalFrame + 1) % FRAMES_IN_FLIGHT;
		++VKR::render::g_Frames;
		nextFrame = backend.GetTime();
		VKR::RM::_Loop();
	}
	mainScene.Cleanup(renderContext.m_LogicDevice);
	backend.Cleanup();
	backend.Shutdown();
	m_endMem();
	return 0;
}

