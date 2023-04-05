#include "vxpch.h"
#include "SceneCamera.h"

namespace Vortex {

	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			VX_CORE_ASSERT(false, "Resizing to 0 is invalid!");
			return;
		}

		m_AspectRatio = (float)width / (float)height;
		m_ViewportSize = { (float)width, (float)height };

		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:
				SetPerspectiveProjectionMatrix(m_PerspectiveFOV, (float)width, (float)height, m_PerspectiveNear, m_PerspectiveFar);
				break;
			case ProjectionType::Orthographic:
				float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
				float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
				float orthoBottom = -m_OrthographicSize * 0.5f;
				float orthoTop = m_OrthographicSize * 0.5f;

				m_ProjectionMatrix = Math::Ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
				break;
		}
	}

	void SceneCamera::SetProjectionType(ProjectionType type)
	{
		m_ProjectionType = type;

		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:
				SetPerspective(m_PerspectiveFOV, m_PerspectiveNear, m_PerspectiveFar);
				break;
			case ProjectionType::Orthographic:
				SetOrthographic(m_OrthographicSize, m_OrthographicNear, m_OrthographicFar);
				break;
		}

		uint32_t width = (uint32_t)m_ViewportSize.x;
		uint32_t height = (uint32_t)m_ViewportSize.y;

		if (width != 0 && height != 0)
		{
			SetViewportSize(width, height);
		}
	}

}
