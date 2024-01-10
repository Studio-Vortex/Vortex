#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Camera.h"

#include "Vortex/Core/TimeStep.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Events/Event.h"
#include "Vortex/Events/MouseEvent.h"

namespace Vortex {

	enum class VORTEX_API CameraMode
	{
		None, FlyCam, ArcBall
	};

	struct VORTEX_API EditorCameraProperties : public PerspectiveProjectionParams
	{
		Math::vec3 Translation;
	};

	class VORTEX_API EditorCamera : public Camera
	{
	public:
		constexpr static float MIN_SPEED{ 0.0005f }, DEFAULT_SPEED{ 0.002f }, MAX_SPEED{ 0.5f };

	public:
		EditorCamera(const EditorCameraProperties& params);
		
		void Focus(const Math::vec3& focusPoint);
		void OnUpdate(TimeStep ts);
		void OnEvent(Event& e);

		VX_FORCE_INLINE bool IsActive() const { return m_IsActive; }
		VX_FORCE_INLINE void SetActive(bool active) { m_IsActive = active; }

		VX_FORCE_INLINE CameraMode GetCurrentMode() const { return m_CameraMode; }

		VX_FORCE_INLINE float GetDistance() const { return m_Distance; }
		VX_FORCE_INLINE void SetDistance(float distance) { m_Distance = distance; }

		VX_FORCE_INLINE const Math::vec3& GetFocalPoint() const { return m_FocalPoint; }

		VX_FORCE_INLINE const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		VX_FORCE_INLINE Math::mat4 GetViewProjection() const { return GetProjectionMatrix() * m_ViewMatrix; }

		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;

		VX_FORCE_INLINE const Math::vec3& GetPosition() const { return m_Position; }

		Math::quaternion GetOrientation() const;

		VX_FORCE_INLINE float GetAspectRatio() const { return m_AspectRatio; }
		VX_FORCE_INLINE float GetPitch() const { return m_Pitch; }
		VX_FORCE_INLINE float GetYaw() const { return m_Yaw; }
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
		Math::mat4 m_ViewMatrix;
		Math::vec3 m_Position, m_Direction, m_FocalPoint;

		bool m_IsActive = false;
		Math::vec2 m_InitialMousePosition{};
		Math::vec3 m_InitialFocalPoint, m_InitialRotation;

		float m_Distance;
		float m_Speed{ DEFAULT_SPEED };

		float m_Pitch, m_Yaw;
		float m_PitchDelta{}, m_YawDelta{};
		Math::vec3 m_PositionDelta{};
		Math::vec3 m_RightDirection{};

		CameraMode m_CameraMode{ CameraMode::ArcBall };

		float m_MinFocusDistance{ 100.0f };

	private:
		friend class EditorLayer;
	};

}
