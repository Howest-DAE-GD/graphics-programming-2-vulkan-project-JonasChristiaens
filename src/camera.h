#pragma once

class Camera {
public:
    Camera(glm::vec3 position, float yaw, float pitch, float fov, float aspect, float near, float far);

    void processKeyboard(int key, float deltaTime);
    void processMouse(float xoffset, float yoffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 getPosition() const { return m_Position; }

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    float m_Yaw;
    float m_Pitch;
    float m_Fov;
    float m_Aspect;
    float m_Near;
    float m_Far;

    float m_MoveSpeed = 5.0f;    // units per second
    float m_MouseSensitivity = 0.1f;

    void updateCameraVectors();
};