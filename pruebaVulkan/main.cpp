// main.cpp: define el punto de entrada de la aplicación de consola.
#include "VKBackend.h"

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	float newFrame = 0.0f;
	float accumulatedTime = 0.0f;
	float deltaTime = 0.0f;
	float currentFrame = 0.0f;
	float frameCap = 0.016f; // 60fps
	int currentLocalFrame = 0;
	backend.Init();
	while (!backend.BackendShouldClose())
	{
		deltaTime = newFrame - currentFrame;
		accumulatedTime += deltaTime;
		backend.PollEvents();
		//m_Scene->Loop(); // Gestionamos si se ha cargado un nuevo modelo a la escena y actualizamos la info de render.
		backend.DrawFrame(currentLocalFrame);
		currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
	}
	backend.Cleanup();
	return 0;
}

