#pragma once
#include "Resource.h"
#include "../core/Objects/VKRModel.h"
#include <assimp/material.h>

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
		inline int tex_type[8] = {
			aiTextureType_BASE_COLOR,
			aiTextureType_DIFFUSE_ROUGHNESS,
			aiTextureType_SPECULAR,
			aiTextureType_AMBIENT,
			aiTextureType_NORMALS,
			aiTextureType_AMBIENT_OCCLUSION,
			aiTextureType_EMISSION_COLOR,
			aiTextureType_LIGHTMAP};
		static RMRequest _RMRequests[256];
		static int _NumRequests = 0;
		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName, render::R_Model* model_ = nullptr);
		void _Init();
		void _Loop();
		void _Destroy();

	}
}
