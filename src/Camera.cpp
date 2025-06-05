#include "pch.h"
#include "Camera.h"

#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <SDL_mouse.h>
#include <iostream>

Camera::Camera(const glm::vec3& origin, float fovAngle, float aspectRatio, float nearPlane, float farPlane)
	: m_Origin{ origin }
	, m_FovAngle{ fovAngle }
	, m_AspectRatio{ aspectRatio }
	, m_NearPlane{ nearPlane }
	, m_FarPlane{ farPlane }
{
	m_FOV = tanf((m_FovAngle * TO_RADIANS) / 2.0f);
}

void Camera::CalculateViewMatrix()
{
	// Method 1
	//ONB -> invViewMatrix
	m_Right = glm::normalize(glm::cross(UnitY, m_Forward));
	m_Up = glm::normalize(glm::cross(m_Forward, m_Right));
	m_InvViewMatrix = { {m_Right, 0}, {m_Up, 0}, {m_Forward, 0}, {m_Origin, 1} };

	//Inverse(ONB) => ViewMatrix
	m_ViewMatrix = glm::inverse(m_InvViewMatrix);
}
void Camera::CalculateProjectionMatrix()
{
	glm::mat4 projection{ CreatePerspectiveFovLH(m_FOV, m_AspectRatio, .1f, 100.f) };

	// combine all space transformation matrix into one matrix
	m_ProjectionMatrix = m_WorldMatrix * m_ViewMatrix * projection;
}

void Camera::Update(const Timer* pTimer)
{
	//Camera Update Logic
	const float deltaTime = pTimer->GetElapsed();
	//std::cout << "Deltatime: " << deltaTime << std::endl;

	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
	MoveCamera(pKeyboardState, deltaTime);
	RotateCamera(pTimer->GetElapsed());

	//Update Matrices
	CalculateViewMatrix();
	CalculateProjectionMatrix();
}

void Camera::SetRotationSpeed(float speed)
{
	m_RotationSpeed = speed;
}

glm::mat4 Camera::CreatePerspectiveFovLH(float fov, float aspect, float zn, float zf)
{
	return
	{
		{1.0f / (aspect * fov), 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f / fov, 0.0f, 0.0f},
		{0.0f, 0.0f, zf / (zf - zn), 1.0f},
		{0.0f, 0.0f, -(zf * zn) / (zf - zn), 0.0f}
	};
}

glm::mat4 Camera::CreateRotationX(float pitch)
{
	return {
		{1, 0, 0, 0},
		{0, glm::cos(pitch), -glm::sin(pitch), 0},
		{0, glm::sin(pitch), glm::cos(pitch), 0},
		{0, 0, 0, 1}
	};
}

glm::mat4 Camera::CreateRotationY(float yaw)
{
	return {
		{glm::cos(yaw), 0, -glm::sin(yaw), 0},
		{0, 1, 0, 0},
		{glm::sin(yaw), 0, glm::cos(yaw), 0},
		{0, 0, 0, 1}
	};
}

void Camera::MoveCamera(const uint8_t* pKeyboardState, float deltaTime)
{
	//std::cout << "MoveCamera function called" << std::endl;
	//Transforming camera's origin ( Movement )
	if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
	{
		//std::cout << "test" << std::endl;
		m_Origin -= m_Right * m_MovementSpeed * deltaTime;
	}
	if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
	{
		m_Origin += m_Right * m_MovementSpeed * deltaTime;
	}
	if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
	{
		m_Origin += m_Forward * m_MovementSpeed * deltaTime;
	}
	if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
	{
		m_Origin -= m_Forward * m_MovementSpeed * deltaTime;
	}
	if (pKeyboardState[SDL_SCANCODE_LSHIFT])
	{
		m_MovementSpeed = 150.0f;
	}
	else
	{
		m_MovementSpeed = 50.0f;
	}
}
void Camera::RotateCamera(float deltaTime)
{
	//Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	//Transforming camera's forward vector ( Rotation )
	if (mouseState == 5)
	{
		m_Origin += m_Up * static_cast<float>(mouseY * -1) * m_MovementSpeed * deltaTime;
	}
	else if (mouseState == 1)
	{
		m_Origin += m_Forward * static_cast<float>(mouseY * -1) * m_MovementSpeed * deltaTime;
		m_TotalYaw += static_cast<float>(mouseX) * m_RotationSpeed * deltaTime;
	}
	else if (mouseState == 4)
	{
		m_TotalYaw += static_cast<float>(mouseX) * m_RotationSpeed * deltaTime;
		m_TotalPitch += static_cast<float>(mouseY * -1) * m_RotationSpeed * deltaTime;

	}
	if (mouseX || mouseY)
	{
		glm::mat4 rotationMatrix{ CreateRotationX(m_TotalPitch * TO_RADIANS) * CreateRotationY(m_TotalYaw * TO_RADIANS) };

		m_Forward = glm::vec3(rotationMatrix * glm::vec4(UnitZ, 0.0f));
		glm::normalize(m_Forward);
	}
}