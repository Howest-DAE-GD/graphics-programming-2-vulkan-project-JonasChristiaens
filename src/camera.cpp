#include "pch.h"
#include "camera.h"

#include <algorithm>

Camera::Camera(glm::vec3 position, float yaw, float pitch, float fov, float aspect, float near, float far)
    : m_Position(position), m_WorldUp(0.0f, 1.0f, 0.0f), m_Yaw(yaw), m_Pitch(pitch),
    m_Fov(fov), m_Aspect(aspect), m_Near(near), m_Far(far)
{
    updateCameraVectors();
}

void Camera::updateCameraVectors()
{
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
    return glm::perspective(glm::radians(m_Fov), m_Aspect, m_Near, m_Far);
}

void Camera::processKeyboard(int key, float deltaTime)
{
    float velocity = m_MoveSpeed * deltaTime;
    if (key == GLFW_KEY_W)
        m_Position += m_Front * velocity;
    if (key == GLFW_KEY_S)
        m_Position -= m_Front * velocity;
    if (key == GLFW_KEY_A)
        m_Position -= m_Right * velocity;
    if (key == GLFW_KEY_D)
        m_Position += m_Right * velocity;
    if (key == GLFW_KEY_SPACE)
        m_Position += m_WorldUp * velocity;
    if (key == GLFW_KEY_LEFT_CONTROL)
        m_Position -= m_WorldUp * velocity;
}

void Camera::processMouse(float xoffset, float yoffset)
{
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    // Clamp pitch
    m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

    updateCameraVectors();
}