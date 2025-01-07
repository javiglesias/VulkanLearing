#include "perfmon/Custom.h"
#include "core/VKRScene.h"
#include "filesystem/ResourceManager.h"
#include "memory/mem_alloc.h"

//std::thread* RMThread;
void _init_resource_manager()
{
	VKR::RM::_Init();
}

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	auto mainScene = VKR::render::GetVKMainScene();
	int currentLocalFrame = 0;
	double currentFrame = 0.f;
	double nextFrame = 0.f;
	double deltaTime = 0.f;
	auto p = NEW(float);
	//RMThread = new std::thread(_init_resource_manager);
	backend.Init();
	mainScene.Init(&backend);
	auto renderContext = VKR::render::GetVKContext();
	currentFrame = backend.GetTime();
	// _initlializePerfmon(&backend);
	while (!backend.BackendShouldClose())
	{
		deltaTime = nextFrame - currentFrame;
		if(deltaTime > 0.016f)
		{
			VKR::render::g_ElapsedTime += backend.GetTime();
			currentFrame = nextFrame;
			VKR::render::g_FrameTime = deltaTime;
			mainScene.DrawScene(&backend, currentLocalFrame);
			currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
			++VKR::render::g_CurrentFrame;
		}
		nextFrame = backend.GetTime();
	}
	mainScene.Cleanup(renderContext.m_LogicDevice);
	backend.Cleanup();
	backend.Shutdown();
	m_endMem();
	return 0;
}

