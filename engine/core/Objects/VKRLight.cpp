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
			m_visual_model = new R_Model("gizmo");
		}

		void Light::Draw(VKBackend* _backend, int _CurrentFrame)
		{
			m_visual_model->Draw(_backend, _CurrentFrame);
		}
	}
}