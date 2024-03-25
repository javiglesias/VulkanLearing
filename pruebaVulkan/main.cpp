#include "VKBackend.h"
#include "Editor.h"

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	VKR::Editor* editor;
	float newFrame = 0.0f;
	float accumulatedTime = 0.0f;
	float deltaTime = 0.0f;
	float currentFrame = 0.0f;
	float frameCap = 0.016f; // 60fps
	int currentLocalFrame = 0;
	backend.Init();
	editor = new VKR::Editor(VKR::render::m_Window, backend.m_Instance, backend.m_Capabilities.minImageCount,
		backend.m_SwapchainImagesCount);
	while (!backend.BackendShouldClose())
	{
		deltaTime = newFrame - currentFrame;
		accumulatedTime += deltaTime;
		backend.PollEvents();
		editor->Loop();
		//m_Scene->Loop(); // Gestionamos si se ha cargado un nuevo modelo a la escena y actualizamos la info de render.
		auto imageIdx = backend.DrawFrame(currentLocalFrame);
		editor->Draw(backend.m_CommandBuffer[currentLocalFrame]);
		backend.SubmitAndPresent(currentLocalFrame, &imageIdx);
		currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
	}
	delete editor;
	backend.Cleanup();
	return 0;
}

