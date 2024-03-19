#pragma once
#include "VKRModel.h"
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
            enum STATUS
            {
                INVALID,
                DIRTY,
	            READY
            };
        public: // VARIABLES
            STATUS m_State;
        public: // FUNCIONES
            Scene();
            void Loop();
            void DrawScene();
            void LoadModel(const char* _filepath, const char* _modelName, glm::vec3 _position,
                glm::vec3 _scale = glm::vec3(1.f), char* _customTexture = nullptr);
            void Cleanup(VkDevice _LogicDevice);
        private:
            R_Model* tempModel;
            std::vector<R_Model*> m_StaticModels;
            std::vector<R_DbgModel*> m_DbgModels;
        private: // FUNCIONES
            void ProcessModelNode(aiNode* _node, const aiScene* _scene, const char* _filepath, char* _customTexture = nullptr);
            void ManageModels();
        };
    }
}