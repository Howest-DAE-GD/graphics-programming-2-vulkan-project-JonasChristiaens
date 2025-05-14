#pragma once

namespace dae
{
	class Camera final
	{
	public:
		Camera(const glm::vec3& origin, float fovAngle, float aspectRatio, float nearPlane, float farPlane);

		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
		void Update(const Timer* pTimer);
		void SetRotationSpeed(float speed);

		glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; };
		glm::mat4 GetWorldMatrix() const { return m_WorldMatrix; }
		glm::vec3 GetOrigin() const { return m_Origin; };
	private:
		glm::mat4 CreatePerspectiveFovLH(float fov, float aspect, float zn, float zf);
		glm::mat4 CreateRotationX(float pitch);
		glm::mat4 CreateRotationY(float yaw);

		void MoveCamera(const uint8_t* pKeyboardState, float deltaTime);
		void RotateCamera(float deltaTime);

		const glm::vec3 UnitX{ 1, 0, 0 };
		const glm::vec3 UnitY{ 0, 1, 0 };
		const glm::vec3 UnitZ{ 0, 0, 1 };
		const glm::vec3 Zero{ 0, 0, 0 };

		glm::vec3 m_Origin{ 0.f, 0.f, 0.f };

		glm::vec3 m_Forward{ 0, 0, 1 };
		glm::vec3 m_Up{ 0, 1, 0 };
		glm::vec3 m_Right{ 1, 0, 0 };

		glm::mat4	m_WorldMatrix{ {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1} };
		glm::mat4	m_InvViewMatrix{};
		glm::mat4	m_ViewMatrix{};
		glm::mat4	m_ProjectionMatrix{};

		float	m_MovementSpeed{ 50.0f };
		float	m_RotationSpeed{ 200.0f };
		float	m_AspectRatio{ 0.0f };
		float	m_FovAngle{ 0.0f };
		float	m_FOV{ 0.0f };
		float	m_TotalPitch{ 0.0f };
		float	m_TotalYaw{ 0.0f };
		float   m_NearPlane{ 0.0f };
		float   m_FarPlane{ 0.0f };
	};
}