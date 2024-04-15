#pragma once
#include "VKBackend.h"
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
            void DrawScene(VKBackend* _backend, int _CurrentFrame);
            void PrepareScene(VKBackend* _backend);
            void PrepareDebugScene(VKBackend* _backend);
            void ShadowPass(VKBackend* _backend, int _CurrentFrame);
            void LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void CreateDebugModel(PRIMITIVE _type);
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Model* tempModel;
            std::vector<R_Model*> m_StaticModels;
            std::vector<R_DbgModel*> m_DbgModels; // lights
        private: // FUNCIONES
            void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture = nullptr);
            void ManageModels(VkDevice _LogicDevice);
        };
        Scene& GetVKMainScene();
    }
}