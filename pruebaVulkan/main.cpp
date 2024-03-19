// main.cpp: define el punto de entrada de la aplicación de consola.
#include "VKBackend.h"
#include <chrono>
// TODO optimizar los commandos que se lanzan al command buffer, haciendolos todos a la vez

int main(int _argc, char** _args)
{
	auto backend = VKR::render::GetVKBackend();
	// Load demo Models
	//char modelPath[512], modelName[64];
	//if (_argc >= 1)
	//{
	//	sprintf(modelPath, "resources/Models/%s/", _args[1]);
	//	sprintf(modelName, "%s", _args[2]);
	//	printf("\n\tApplication launched with Params(%s): %s", modelPath, modelName);
	//	backend.LoadModel(modelPath, modelName, glm::vec3(0.f, 2.f, 0.f),
	//		glm::vec3(0.5f, 0.5f, 0.5f));
	//	//// FLOOR
	//	//sprintf(modelPath, "resources/Models/Plane/glTF/");
	//	//sprintf(modelName, "Plane.gltf");
	//	//backend.LoadModel(modelPath, modelName, glm::vec3(0.f), 
	//	//	glm::vec3(3.f, 1.f, 3.f));

	//	//sprintf(modelPath, "resources/Models/Cube/glTF/");
	//	//sprintf(modelName, "Cube.gltf");
	//	//char _texture[64] = { "resources/textures/cubemaps/office.png" };
	//	//backend.LoadModel(modelPath, modelName, glm::vec3(0.f),
	//	//	glm::vec3(13.f, 13.f, 13.f), _texture);
	//}
	//else
	//{
	//	exit(-969);
	//}
	backend.Init();
	backend.Loop();
	backend.Cleanup();
	return 0;
}

