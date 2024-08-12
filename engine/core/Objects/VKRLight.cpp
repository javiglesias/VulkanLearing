#include "VKRLight.h"
#include "../../video/VKBackend.h"
#include "VKRModel.h"

namespace VKR 
{
	namespace render
	{
		Light::Light()
		{
			auto tempModel = new R_DbgModel(ARROW);
			tempModel->m_Material.m_Texture = new Texture();
			m_DbgModels.push_back(tempModel);
		}
		Directional::Directional()
		{
			auto tempModel = new R_DbgModel(ARROW);
			tempModel->m_Material.m_Texture = new Texture();
			m_DbgModels.push_back(tempModel);
		}
	}
}