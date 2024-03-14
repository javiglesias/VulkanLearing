// main.cpp: define el punto de entrada de la aplicación de consola.
#include "VKBackend.h"
#include <chrono>
// TODO optimizar los commandos que se lanzan al command buffer, haciendolos todos a la vez

int main(int _argc, char** _args)
{

	float newFrame = 0.0f;
	float accumulatedTime = 0.0f;
	float deltaTime = 0.0f;
	float currentFrame = 0.0f;
	float frameCap = 0.016f; // 60fps
	int currentLocalFrame = 0;

	auto backend = VKR::render::GetVKBackend();
	// Load demo Models
	char modelPath[512], modelName[64];
	if (_argc >= 1)
	{
		sprintf(modelPath, "resources/Models/%s/", _args[1]);
		sprintf(modelName, "%s", _args[2]);
		printf("\n\tApplication launched with Params(%s): %s", modelPath, modelName);
		backend.LoadModel(modelPath, modelName);
		// FLOOR
		sprintf(modelPath, "resources/Models/Plane/glTF/");
		sprintf(modelName, "Plane.gltf");
		backend.LoadModel(modelPath, modelName);
	}
	else
	{
		exit(-969);
	}
	backend.Init();
	while (!backend.BackendShouldClose())
	{
		backend.PollEvents();
		// Draw a Frame!
		backend.EditorLoop();
		//newFrame = static_cast<float>(glfwGetTime());
		deltaTime = newFrame - currentFrame;
		accumulatedTime += deltaTime;
		//if (accumulatedTime >= frameCap) // Render frame
		//{
		//	accumulatedTime = 0.0f;
			backend.DrawFrame(currentLocalFrame);
			currentLocalFrame = (currentLocalFrame + 1) % VKR::render::FRAMES_IN_FLIGHT;
		 //}
		//currentFrame = static_cast<float>(glfwGetTime());
	}
	backend.Cleanup();
	return 0;
}

