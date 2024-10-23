#ifndef _C_SCENE
#define _C_SCENE

#include <thread>

#include "../video/VKBufferObjects.h"
#include "Objects/VKRModel.h"

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VKR
{
    namespace render
    {
        class VKBackend;
        struct R_Model;
        class R_Cubemap;

        class Scene
        {
        public: // FUNCIONES
            Scene() {}
            void Init(VKBackend* _backend);
            void Loop();
            void ReloadShaders(VKBackend* _backend);
            void DrawScene(VKBackend* _backend, int _CurrentFrame);
            void PrepareCubemapScene(VKBackend* _backend);
            void PrepareScene(VKBackend* _backend);
            void PrepareDebugScene(VKBackend* _backend);
            void ShadowPass(VKBackend* _backend, int _CurrentFrame);
            void GeometryPass(VKBackend* _backend, int _CurrentFrame);
            void LoadCubemapModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Cubemap* m_Cubemap;
            std::vector<LightBufferObject> m_LightsOs;
        private: // FUNCIONES
            void DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment);
        };
        Scene& GetVKMainScene();
    }
}
#endif