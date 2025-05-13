#pragma once

#include "HelperClasses/Math.h"

namespace dae
{
	class Camera final
	{
	public:
		Camera(const Vector3& origin, float fovAngle, float aspectRatio, float nearPlane, float farPlane);

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
		void Update(const Timer* pTimer);
		void RotateModel(const Timer* pTimer);
		void SetRotationSpeed(float speed);

		Matrix GetViewMatrix() const;
		Matrix GetProjectionMatrix() const;
		Matrix GetWorldMatrix() const;
		Vector3 GetOrigin() const;
	private:
		void MoveCamera(const uint8_t* pKeyboardState, float deltaTime);
		void RotateCamera(float deltaTime);

		float	m_MovementSpeed{ 50.0f };
		float	m_RotationSpeed{ 200.0f };
		float	m_AspectRatio{ 0.0f };
		float	m_FovAngle{ 0.0f };
		float	m_FOV{ 0.0f };
		float	m_TotalPitch{ 0.0f };
		float	m_TotalYaw{ 0.0f };
		float   m_NearPlane{ 0.0f };
		float   m_FarPlane{ 0.0f };
		float	m_ConstantRotation{ 0.0f };

		Vector3 m_Origin{ 0.f, 0.f, 0.f };

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		Matrix	m_WorldMatrix{ {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} };
		Matrix	m_InvViewMatrix{};
		Matrix	m_ViewMatrix{};
		Matrix	m_ProjectionMatrix{};
	};
}