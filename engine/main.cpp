#include "perfmon/Custom.h"
#include "core/VKRScene.h"
#include "filesystem/ResourceManager.h"

std::thread* RMThread;
void _init_resource_manager()
{
	VKR::RM::_Init();
}

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	auto mainScene = VKR::render::GetVKMainScene();
	int currentLocalFrame = 0;
	float currentFrame = 0.f;
	float nextFrame = 0.f;
	float deltaTime = 0.f;
	RMThread = new std::thread(_init_resource_manager);
	backend.Init();
	mainScene.Init(&backend);
	auto renderContext = VKR::render::GetVKContext();
	// mainScene.LoadCubemapModel("resources/models/Box/glTF/", "Box.gltf", glm::vec3(0.f, 1.f, 0.f));
	// mainScene.PrepareCubemapScene(&backend);
	currentFrame = backend.GetTime();
	// _initlializePerfmon(&backend);
	while (!backend.BackendShouldClose())
	{
		deltaTime = nextFrame - currentFrame;
		if(deltaTime > 0.016f)
		{
			VKR::render::g_ElapsedTime += backend.GetTime();
			currentFrame = nextFrame;
			VKR::render::g_DeltaTime = deltaTime;
			VKR::render::g_FrameTime[VKR::render::g_CurrentFrameTime] = deltaTime;
			++VKR::render::g_CurrentFrameTime %= VKR::render::g_FrameGranularity;
			mainScene.DrawScene(&backend, currentLocalFrame);
			currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
			++VKR::render::g_CurrentFrame;
		}
		nextFrame = backend.GetTime();
	}
	mainScene.Cleanup(renderContext.m_LogicDevice);
	backend.Cleanup();
	backend.Shutdown();
	return 0;
}

