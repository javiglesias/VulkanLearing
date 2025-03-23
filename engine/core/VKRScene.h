#ifndef _C_SCENE
#define _C_SCENE

#include "../video/VKBackend.h"

namespace VKR
{
    namespace render
    {
        struct R_Model;
        class R_Cubemap;

        class Scene
        {
        public: // FUNCIONES
            Scene() {}
            void Init(VKBackend* _backend);
            void Update();
            void ReloadShaders(VKBackend* _backend);
            void DrawScene(VKBackend* _backend, int _CurrentFrame);
            void PrepareCubemapScene(VKBackend* _backend);
            void PrepareScene(VKBackend* _backend);
            //void PrepareDebugScene(VKBackend* _backend);
            void ShadowPass(VKBackend* _backend, int _CurrentFrame);
            void GeometryPass(VKBackend* _backend, int _CurrentFrame);
            void LoadCubemapModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Cubemap* m_Cubemap;
            
        private: // FUNCIONES
            void DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment);
        };
        Scene& GetVKMainScene();
    }
}
#endif