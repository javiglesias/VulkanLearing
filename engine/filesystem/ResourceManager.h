#pragma once
#include "Resource.h"
#include "../core/Objects/VKRModel.h"
#define RESOURCES_PATH "resources/"
#define MODELS_PATH "resources/models/"

namespace VKR
{
	namespace RM
	{
		struct RMRequest
		{
			TYPE type;
			char filepath[256];
			char resourceName[64];
			void* modelPtr;
		};

		static RMRequest _RMRequests[256];
		static int _NumRequests = 0;
		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName, render::R_Model* model_ = nullptr);
		void _Init();
		void _Loop();
		void _Destroy();

	}
}
