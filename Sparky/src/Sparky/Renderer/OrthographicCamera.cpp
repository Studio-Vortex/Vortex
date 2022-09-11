#include "sppch.h"
#include "OrthographicCamera.h"

namespace Sparky {

	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		:
		m_ProjectionMatrix(Math::Ortho(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(Math::Identity())
	{
		SP_PROFILE_FUNCTION();

		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		SP_PROFILE_FUNCTION();

		m_ProjectionMatrix = Math::Ortho(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewMatrix = Math::Identity();
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	void OrthographicCamera::ReCalculateViewMatrix()
	{
		SP_PROFILE_FUNCTION();

		Math::mat4 transform = Math::Translate(m_Position) * Math::Rotate(m_Rotation, { 0.0f, 0.0f, 1.0f });

		m_ViewMatrix = Math::Inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

}