#include "VKBackend.h"
#include "Editor.h"
#include "VKRScene.h"

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	VKR::Editor* editor;
	VKR::render::Scene* mainScene = new VKR::render::Scene();
	float newFrame = 0.0f;
	float accumulatedTime = 0.0f;
	float deltaTime = 0.0f;
	float currentFrame = 0.0f;
	float frameCap = 0.016f; // 60fps
	int currentLocalFrame = 0;
	backend.Init();
	editor = new VKR::Editor(VKR::render::m_Window, backend.m_Instance, backend.m_Capabilities.minImageCount,
		backend.m_SwapchainImagesCount);
	auto renderContext = VKR::render::GetVKContext();
	while (!backend.BackendShouldClose())
	{
		deltaTime = newFrame - currentFrame;
		accumulatedTime += deltaTime;
		if(VKR::render::m_CreateTestModel)
		{
			mainScene->LoadModel("resources/models/Sponza/glTF/", "Sponza.gltf", glm::vec3(1.f));
			VKR::render::m_CreateTestModel = false;
		}
		backend.PollEvents();
		editor->Loop();
		//m_Scene->Loop(); // Gestionamos si se ha cargado un nuevo modelo a la escena y actualizamos la info de render.
		auto imageIdx = backend.DrawFrame(currentLocalFrame);
		mainScene->DrawScene(&backend, currentLocalFrame);
		editor->Draw(backend.m_CommandBuffer[currentLocalFrame]);
		backend.SubmitAndPresent(currentLocalFrame, &imageIdx);
		currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
	}
	mainScene->Cleanup(renderContext.m_LogicDevice);
	editor->Cleanup();
	backend.Cleanup();
	editor->Shutdown();
	backend.Shutdown();
	return 0;
}

