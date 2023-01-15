#pragma once

#include "Vortex/Renderer/Camera.h"

namespace Vortex {

	class SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

	public:
		SceneCamera() = default;
		~SceneCamera() override = default;

		void SetPerspective(float verticalFOV, float nearClip = 0.01f, float farClip = 1000.0f);
		void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

		void SetViewportSize(uint32_t width, uint32_t height);
		inline const Math::vec2& GetViewportSize() const { return m_ViewportSize; }

		inline float GetPerspectiveVerticalFOVRad() const { return m_PerspectiveFOV; }
		inline void SetPerspectiveVerticalFOVRad(float verticalFOV) { m_PerspectiveFOV = verticalFOV; }
		inline float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		inline void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; }
		inline float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		inline void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; }

		inline float GetOrthographicSize() const { return m_OrthographicSize; }
		inline void SetOrthographicSize(float size) { m_OrthographicSize = size; }
		inline float GetOrthographicNearClip() const { return m_OrthographicNear; }
		inline void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; }
		inline float GetOrthographicFarClip() const { return m_OrthographicFar; }
		inline void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; }

		inline ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type);

	private:
		ProjectionType m_ProjectionType = ProjectionType::Orthographic;

		float m_PerspectiveFOV = Math::Deg2Rad(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		Math::vec2 m_ViewportSize = {};

		float m_AspectRatio = 0.0f;
	};

}
