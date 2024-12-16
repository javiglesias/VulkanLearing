#include "VKRLight.h"
#include "../../video/VKBackend.h"
#include "VKRModel.h"
#include "../../filesystem/gltfReader.h"

namespace VKR 
{
	namespace render
	{
		Light::Light()
		{
			m_LightVisual = new R_Model("resources/models/Box/glTF/", "Box.gltf");
		}
		void Light::Cleanup(VkDevice _LogicDevice)
		{
			//m_LightVisual->Cleanup(_LogicDevice);
			delete(m_LightVisual);
		}
	}
}