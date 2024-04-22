#pragma once
#include "VKBackend.h"
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
            Scene();
            void Loop();
            void ReloadShaders(VKBackend* _backend);
            void DrawScene(VKBackend* _backend, int _CurrentFrame);
            void PrepareCubemapScene(VKBackend* _backend);
            void PrepareScene(VKBackend* _backend);
            void PrepareDebugScene(VKBackend* _backend);
            void ShadowPass(VKBackend* _backend, int _CurrentFrame);
            void LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void CreateDebugModel(PRIMITIVE _type);
            void CreateCubemap();
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Model* tempModel;
            R_Cubemap* m_Cubemap;
            std::vector<R_Model*> m_StaticModels;
            std::vector<R_DbgModel*> m_DbgModels; // lights
        private: // FUNCIONES
            void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture = nullptr);
        };
        Scene& GetVKMainScene();
    }
}