#pragma once
#include "Resource.h"

namespace VKR
{
	namespace RM
	{
		struct RMRequest
		{
			TYPE type;
			const char* filepath;
			const char* resourceName;
		};
		static RMRequest _RMRequests[256];
		static int _NumRequests = 0;
		void _AddRequest(TYPE _type, const char* _filepath, const char* _resourceName);
		void _Init();
		void _Loop();
		void _Destroy();

	}
}
