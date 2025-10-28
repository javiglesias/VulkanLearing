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
		}

		void Directional::Init()
		{
			Light::Init();
		}

		void Light::Draw(VKBackend* _backend, int _CurrentFrame)
		{
		}
		void Light::Prepare(VKBackend* _backend)
		{
		}

		void Point::Init()
		{
			Light::Init();
		}
	}
}