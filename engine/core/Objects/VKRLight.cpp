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
			m_LightVisual = new R_DbgModel("resources/models/Cube/glTF/", "Cube.gltf");
			m_DbgModels.push_back(m_LightVisual);
		}
	}
}