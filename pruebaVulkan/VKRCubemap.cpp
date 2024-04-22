#include "VKRCubemap.h"

namespace VKR
{
	namespace render
	{
		R_Cubemap::R_Cubemap(std::string _texturePath)
		{
			strcpy(m_Path, _texturePath.c_str());	
			m_Material = new R_CubemapMaterial(_texturePath);
		}
	}
}