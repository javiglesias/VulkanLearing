#pragma once
#include "Resource.h"
#include "../core/Objects/VKRModel.h"
#define RESOURCES_PATH "resources/"
#define MODELS_PATH "resources/models/"
#define MAPS_PATHS "resources/world"

namespace VKR
{
	namespace RM
	{
		//request type
		enum REQ_TYPE
		{
			INVALID = -1,
			LOAD,
			SAVE
		};
		// this object is the one that is created when a request is made.
		struct RMRequest
		{
			REQ_TYPE type= INVALID;
			RES_TYPE res_type = RES_TYPE::UNDEFINED;
			char filepath[256];
			char resourceName[64];
			void* modelPtr;
			RMRequest(const char* _PathFromLoad);
			RMRequest(){}
		};

		void _AddRequest(REQ_TYPE _type, RES_TYPE _res_type, const char* _filepath = nullptr, const char* _resourceName=nullptr, render::R_Model* model_ = nullptr);
		void _Init();
		void _Loop();
		void _Destroy();
		void _SaveResources();
	}
}
