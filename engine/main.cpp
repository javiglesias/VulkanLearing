#include "video/VKBackend.h"
#include "core/Editor.h"
#include "core/VKRScene.h"

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	VKR::render::Editor* editor;
	auto mainScene = VKR::render::GetVKMainScene();
	int currentLocalFrame = 0;
	
	backend.Init();
	mainScene.Init();
	editor = new VKR::render::Editor(VKR::render::m_Window, backend.m_Instance, backend.m_Capabilities.minImageCount,
		backend.m_SwapchainImagesCount);
	auto renderContext = VKR::render::GetVKContext();
	mainScene.LoadCubemapModel("resources/models/Box/glTF/", "Box.gltf", glm::vec3(0.f, 1.f, 0.f));
	mainScene.PrepareCubemapScene(&backend);
	while (!backend.BackendShouldClose())
	{
		backend.StartPerfFrame();
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
		backend.EndPerfFrame();
	}
	mainScene.Cleanup(renderContext.m_LogicDevice);
	editor->Cleanup();
	backend.Cleanup();
	editor->Shutdown();
	backend.Shutdown();
	return 0;
}

