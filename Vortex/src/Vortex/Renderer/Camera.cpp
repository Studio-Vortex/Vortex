#include "vxpch.h"
#include "Camera.h"

namespace Vortex {

	Camera::Camera(const Math::mat4& projection, const Math::mat4& unreversedProjection)
		: m_ProjectionMatrix(projection), m_UnreversedProjectionMatrix(unreversedProjection) { }

	Camera::Camera(float FOVdeg, float width, float height, float nearClip, float farClip)
		:
		m_ProjectionMatrix(Math::PerspectiveFOV(Math::Deg2Rad(FOVdeg), width, height, nearClip, farClip)),
		m_UnreversedProjectionMatrix(Math::PerspectiveFOV(Math::Deg2Rad(FOVdeg), width, height, nearClip, farClip)) { }

	void Camera::SetProjectionMatrix(const Math::mat4& projection, const Math::mat4& unreversedProjection)
	{
		m_ProjectionMatrix = projection;
		m_UnreversedProjectionMatrix = unreversedProjection;
	}

	void Camera::SetPerspectiveProjectionMatrix(float FOVrad, float width, float height, float nearClip, float farClip)
	{
		m_ProjectionMatrix = Math::PerspectiveFOV(FOVrad, width, height, nearClip, farClip);
		m_UnreversedProjectionMatrix = Math::PerspectiveFOV(FOVrad, width, height, nearClip, farClip);
	}

}
