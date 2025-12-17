#pragma once

#include <glm/vec3.hpp>

struct 
raiCamera
{
    glm::vec3 g_CameraPos = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 g_CameraDefPos = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 g_CameraForward = glm::vec3(-1.f, 0.f, 0.f);
    glm::vec3 g_CameraUp = glm::vec3(0.f, 1.f, 0.f);
    double m_CameraYaw = 0.f, 
        m_CameraPitch = 0.f;
    float m_CameraSpeed = 0.6f;
    float m_CameraFOV = 70.f;
};

inline raiCamera camera;
