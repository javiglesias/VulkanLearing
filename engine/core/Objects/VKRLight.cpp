#include "VKRLight.h"
#include "../../video/VKBackend.h"
#include "VKRModel.h"
#include "../../filesystem/gltfReader.h"

namespace VKR 
{
	namespace render
	{
		void Light::Init()
		{
			m_visual_model = new R_Model("BoxVertexColors");
		}

		void Directional::Init()
		{
			Light::Init();
		}

		void Light::Draw(VKBackend* _backend, int _CurrentFrame)
		{
			m_visual_model->Draw(_backend, _CurrentFrame);
		}

		void Point::Init()
		{
			Light::Init();
		}
	}
}