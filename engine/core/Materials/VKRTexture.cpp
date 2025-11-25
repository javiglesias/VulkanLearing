#include "VKRTexture.h"
#include "../../video/VKRUtils.h"
#include "../../perfmon/Custom.h"
#include <signal.h>
#include <string>
#include <cmath>


namespace VKR
{
	namespace render
	{
		void Texture::init(std::string _path)
		{
			memset(m_Path, 0, 256);
			if (_path.empty())
				sprintf(m_Path, "resources/Textures/defaultMissing.png");
			else
				sprintf(m_Path, "%s", _path.c_str());
		}

		Texture::Texture(vk_Allocated_Image _image, VkSampler _sampler)
		{
			vk_image = _image;
			m_Sampler = _sampler;
		}
	}
}
