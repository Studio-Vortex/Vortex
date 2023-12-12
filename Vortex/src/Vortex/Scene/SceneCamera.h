#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Camera.h"

#include "Vortex/Math/Ray.h"

namespace Vortex {

	class VORTEX_API SceneCamera : public Camera
	{
	public:
		enum class ProjectionType { Perspective = 0, Orthographic = 1 };

	public:
		SceneCamera() = default;
		~SceneCamera() override = default;

		void SetPerspective(float verticalFOV, float nearClip = 0.01f, float farClip = 1000.0f);
		void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

		void SetViewportSize(uint32_t width, uint32_t height);
		VX_FORCE_INLINE const Math::vec2& GetViewportSize() const { return m_ViewportSize; }

		VX_FORCE_INLINE float GetPerspectiveVerticalFOVRad() const { return m_PerspectiveFOV; }
		VX_FORCE_INLINE void SetPerspectiveVerticalFOVRad(float verticalFOV) { m_PerspectiveFOV = verticalFOV; }
		VX_FORCE_INLINE float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		VX_FORCE_INLINE void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; }
		VX_FORCE_INLINE float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		VX_FORCE_INLINE void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; }

		VX_FORCE_INLINE float GetOrthographicSize() const { return m_OrthographicSize; }
		VX_FORCE_INLINE void SetOrthographicSize(float size) { m_OrthographicSize = size; }
		VX_FORCE_INLINE float GetOrthographicNearClip() const { return m_OrthographicNear; }
		VX_FORCE_INLINE void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; }
		VX_FORCE_INLINE float GetOrthographicFarClip() const { return m_OrthographicFar; }
		VX_FORCE_INLINE void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; }

		VX_FORCE_INLINE ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type);

		Math::Ray Raycast(const Math::vec2& point, const Math::vec3& cameraPosition, float maxDistance, const Math::mat4& view) const;
		Math::vec3 ScreenPointToWorldPoint(const Math::vec2& point, const Math::vec2& viewportMinBound, const Math::vec3& cameraPosition, const float maxDistance, const Math::mat4& view) const;
		Math::vec2 ScreenPointToViewportPoint(const Math::vec2& point) const;

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
