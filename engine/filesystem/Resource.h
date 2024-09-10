#ifndef _C_RESOURCE
#define _C_RESOURCE
#include <cstring>
namespace VKR
{
	enum STATE
	{
		LOADING,
		READY,
		COUNT
	};
	enum TYPE
	{
		UNDEFINED=-1,
		STATIC_MODEL,
		DBG_MODEL,
		TEXTURE,
		SOUND
	};
	struct sResource
	{
		STATE m_State{LOADING};
		TYPE m_Type{UNDEFINED};
		char m_PathFromLoad[256];
		void (*m_LoadFunc)(const char*);
		sResource() {}
		sResource(const char* _PathFromLoad)
		{
			memcpy(m_PathFromLoad, _PathFromLoad, sizeof(m_PathFromLoad));
		}
	};
}
#endif