#ifndef _C_RESOURCE
#define _C_RESOURCE
#include <cstring>
namespace VKR
{
	// resource requested state
	enum RES_STATE
	{
		LOADING,
		READY
	};

	// resource type to manage
	enum RES_TYPE
	{
		UNDEFINED=-1,
		STATIC_MODEL,
		ASSIMP_MODEL,
		DBG_MODEL,
		TEXTURE,
		SOUND,
		MAP_FILE
	};

	struct sResource
	{
		RES_STATE m_State{LOADING};
		RES_TYPE m_Type{UNDEFINED};
		char m_PathFromLoad[256];
	};
}
#endif