#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Camera.h"

#include "Vortex/Core/TimeStep.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Events/Event.h"
#include "Vortex/Events/MouseEvent.h"

namespace Vortex {

	enum class VORTEX_API CameraMode
	{
		None, FlyCam, ArcBall
	};

	class VORTEX_API EditorCamera : public Camera
	{
	public:
		EditorCamera(const float degFOV, const float width, const float height, const float nearP, const float farP);
		
		void Init();

		void Focus(const Math::vec3& focusPoint);
		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);

		bool IsActive() const { return m_IsActive; }
		void SetActive(bool active) { m_IsActive = active; }

		bool IsUsing2DView() const { return m_Use2DView; }
		void SetUse2DView(bool use) { m_Use2DView = use; }

		bool IsUsingTopDownView() const { return m_UseTopDownView; }
		void SetUseTopDownView(bool use) { m_UseTopDownView = use; }

		CameraMode GetCurrentMode() const { return m_CameraMode; }

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		const Math::vec3& GetFocalPoint() const { return m_FocalPoint; }

		inline void SetViewportSize(uint32_t width, uint32_t height)
		{
			if (m_ViewportWidth == width && m_ViewportHeight == height)
				return;

			SetPerspectiveProjectionMatrix(m_VerticalFOV, (float)width, (float)height, m_NearClip, m_FarClip);

			m_ViewportWidth = width;
			m_ViewportHeight = height;
		}

		const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Math::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_ViewMatrix; }

		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;

		const Math::vec3& GetPosition() const { return m_Position; }

		Math::quaternion GetOrientation() const;

		float GetVerticalFOV() const { return m_VerticalFOV; }
		void SetVerticalFOV(float degFOV);

		float GetAspectRatio() const { return m_AspectRatio; }
		float GetNearClip() const { return m_NearClip; }
		float GetFarClip() const { return m_FarClip; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetCameraSpeed() const;

	private:
		void UpdateCameraView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const Math::vec2& delta);
		void MouseRotate(const Math::vec2& delta);
		void MouseZoom(float delta);

		Math::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;

	private:
		constexpr static float MIN_SPEED{ 0.0005f }, MAX_SPEED{ 2.0f };

	private:
		Math::mat4 m_ViewMatrix;
		Math::vec3 m_Position, m_Direction, m_FocalPoint;

		// Perspective projection params
		float m_VerticalFOV, m_AspectRatio, m_NearClip, m_FarClip;

		bool m_IsActive = false;
		bool m_Use2DView = false;
		bool m_UseTopDownView = false;
		bool m_Panning, m_Rotating;
		Math::vec2 m_InitialMousePosition{};
		Math::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_NormalSpeed{ 0.002f };

		float m_Pitch, m_Yaw;
		float m_PitchDelta{}, m_YawDelta{};
		Math::vec3 m_PositionDelta{};
		Math::vec3 m_RightDirection{};

		CameraMode m_CameraMode{ CameraMode::ArcBall };

		float m_MinFocusDistance{ 100.0f };

		uint32_t m_ViewportWidth{ 1280 }, m_ViewportHeight{ 720 };

	private:
		friend class EditorLayer;
	};

}
