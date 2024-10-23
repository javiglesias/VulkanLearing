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
			m_LightVisual = new R_DbgModel("resources/models/Box/glTF/", "Box.gltf");
			m_ModelMatrix = glm::scale(glm::mat4(1.f), glm::vec3(m_DebugScale));
		}
		void Light::Cleanup(VkDevice _LogicDevice)
		{
			m_LightVisual->Cleanup(_LogicDevice);
		}
	}
}