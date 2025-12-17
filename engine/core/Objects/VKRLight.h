#ifndef _C_LIGHT
#define _C_LIGHT

#include "VKRModel.h"

#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace VKR 
{
	namespace render
	{
		struct R_DbgModel;
		enum LightType
		{
			LIGHT_DIRECTIONAL,
			LIGHT_POINT,
			LIGHT_SPOT,
			LIGHT_OMNI
		};
		struct Light
		{
			LightType m_Type{LIGHT_POINT};
			bool m_Editable = false;
			float m_ShadowCameraFOV;
			float m_ShadowAspectRatio;
			float m_ShadowBias = 0.0025f;
			float m_DebugScale = 0.1f;
			glm::vec3 m_Pos {0.f, 0.1f, 0.f};
			glm::vec3 m_Color {1.f, 0.f, 1.f};

			void Draw(VKBackend* _backend, int _CurrentFrame);
			void Prepare(VKBackend* _backend);
			virtual void Init();
		};
		
		struct Directional : public Light
		{
			void Init() override;
			float m_Depth 	= 10.f;
			float m_Right 	= 10.f;
			float m_Up      = 10.f;
			glm::vec3 m_UpVector = glm::vec3(0.f, 1.f, 0.f);
			glm::vec3 m_Center 		= glm::vec3(0.f, 0.f, 0.f);
		};
		struct Point : public Light
		{
			void Init() override;
			// PointLight
			float m_Kc = 1.0f;
			float m_Kl = 0.7f;
			float m_Kq = 1.8f;
		};
		struct Spot : public Light
		{
			void Init() override {}
		};
		struct Omni : public Light
		{
			void Init() override {}
		};
	}
}
#endif