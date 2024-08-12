#include "perfmon/Custom.h"
#include "core/Editor/Editor.h"
#include "core/VKRScene.h"

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	VKR::render::Editor* editor;
	auto mainScene = VKR::render::GetVKMainScene();
	int currentLocalFrame = 0;
	float currentFrame = 0.f;
	float nextFrame = 0.f;
	float deltaTime = 0.f;
	
	backend.Init();
	mainScene.Init(&backend);
	editor = new VKR::render::Editor(VKR::render::m_Window, backend.m_Instance, backend.m_Capabilities.minImageCount,
		backend.m_SwapchainImagesCount);
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
			if(VKR::render::m_SceneDirty)
			{
				mainScene.PrepareScene(&backend);
				VKR::render::m_SceneDirty = false;
			}
			backend.PollEvents();
			editor->Loop(&mainScene, &backend);
			//m_Scene->Loop(); // Gestionamos si se ha cargado un nuevo modelo a la escena y actualizamos la info de render.
			auto imageIdx = backend.DrawFrame(currentLocalFrame);
			mainScene.DrawScene(&backend, currentLocalFrame);
			editor->Draw(backend.m_CommandBuffer[currentLocalFrame]);
			backend.EndRenderPass(currentLocalFrame);
			backend.SubmitAndPresent(currentLocalFrame, &imageIdx);
			currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
			++VKR::render::g_CurrentFrame;
		}
		nextFrame = backend.GetTime();
	}
	mainScene.Cleanup(renderContext.m_LogicDevice);
	editor->Cleanup();
	backend.Cleanup();
	editor->Shutdown();
	backend.Shutdown();
	return 0;
}

