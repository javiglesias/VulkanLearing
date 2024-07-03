#pragma once
#include "../video/VKBackend.h"
#include "VKRCubemap.h"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VKR
{
    namespace render
    {
        class Scene
        {
        public: // FUNCIONES
            Scene() {}
            void Init();
            void Loop();
            void ReloadShaders(VKBackend* _backend);
            void DrawScene(VKBackend* _backend, int _CurrentFrame);
            void PrepareCubemapScene(VKBackend* _backend);
            void PrepareScene(VKBackend* _backend);
            void PrepareDebugScene(VKBackend* _backend);
            void ShadowPass(VKBackend* _backend, int _CurrentFrame);
            void GeometryPass(VKBackend* _backend, int _CurrentFrame);
            R_Model* LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void LoadStaticModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void LoadCubemapModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void CreateDebugModel(PRIMITIVE _type);
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Model* tempModel;
            R_Cubemap* m_Cubemap;
        private: // FUNCIONES
            void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture = nullptr);
            void DrawCubemapScene(VKBackend* _backend, int _CurrentFrame, glm::mat4 _projection, glm::mat4 _view, uint32_t _dynamicAlignment);
        };
        Scene& GetVKMainScene();
    }
}