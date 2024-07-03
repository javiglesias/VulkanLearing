#include "VKRLight.h"
#include "../video/VKBackend.h"

namespace VKR 
{
	namespace render
	{
		Light::Light()
		{
			auto tempModel = new R_DbgModel(SPHERE);
			tempModel->m_Material.m_Texture = new Texture();
			m_DbgModels.push_back(tempModel);
		}
		void Light::AddCommand()
		{

		}
	}
}