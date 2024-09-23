#ifndef _C_LIGHT
#define _C_LIGHT

#include "VKRModel.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
namespace VKR 
{
	namespace render
	{
		class R_DbgModel;
		enum LightType
		{
			LIGHT_DIRECTIONAL,
			LIGHT_POINT,
			LIGHT_SPOT,
			LIGHT_OMNI
		};
		struct Light
		{
		public:
			bool m_Editable = false;
			float m_DebugScale;
			glm::vec3 m_Pos {0.f, 0.f, 1.f};
			glm::vec3 m_Color {1.f};
		private: // Variables
			LightType m_Type{LIGHT_POINT};
			float m_ShadowCameraFOV;
			float m_ShadowAspectRatio;
			float m_ShadowBias = 0.0025f;
			R_DbgModel* m_LightVisual;
		public: // Functions
			Light();
		};
		
		struct Directional : public Light
		{
			float m_Right 	  = 300.f;
			float m_Depth 	= 300.f;
			float m_Up        	= 300.f;
			glm::vec3 m_UpVector = glm::vec3(0.f, -1.f, 0.f);
			glm::vec3 m_Center 		= glm::vec3(0.f, 0.f, -1.f);
		};
		struct Point : public Light
		{
			// PointLight
			float m_Kc = 1.0f;
			float m_Kl = 0.7f;
			float m_Kq = 1.8f;
		};
		struct Spot : public Light
		{
		};
		struct Omni : public Light
		{
		};
	}
}
#endif