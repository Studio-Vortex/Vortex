#include "vxpch.h"
#include "Camera.h"

namespace Vortex {

	Camera::Camera(const PerspectiveProjectionParams& params)
	{
		SetPerspective(params.FOV, params.NearClip, params.FarClip);
		SetViewportSize(params.Width, params.Height);
	}

	Camera::Camera(const OrthographicProjectionParams& params)
	{
		SetOrthographic(params.OrthoSize, params.NearClip, params.FarClip);
		SetViewportSize(params.Width, params.Height);
	}

	void Camera::SetProjectionMatrix(const Math::ProjectionParams& params)
	{
		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:  m_ProjectionMatrix = params.CalculatePerspectiveProjectionMatrix();  break;
			case ProjectionType::Orthographic: m_ProjectionMatrix = params.CalculateOrthographicProjectionMatrix(); break;
		}

		m_IsDirty = true;
	}

    void Camera::SetProjectionType(ProjectionType type)
    {
		m_ProjectionType = type;

		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:
			{
				SetPerspective(m_PerspectiveFOV, m_PerspectiveNear, m_PerspectiveFar);
				break;
			}
			case ProjectionType::Orthographic:
			{
				SetOrthographic(m_OrthographicSize, m_OrthographicNear, m_OrthographicFar);
				break;
			}
		}

		m_IsDirty = true;
    }

	void Camera::SetViewportSize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
		{
			VX_CORE_ASSERT(false, "Resizing to 0 is invalid!");
			return;
		}

		m_AspectRatio = (float)width / (float)height;
		m_ViewportSize = { (float)width, (float)height };

		Math::ProjectionParams params;
		params.Width = m_ViewportSize.x;
		params.Height = m_ViewportSize.y;

		switch (m_ProjectionType)
		{
			case ProjectionType::Perspective:
			{
				params.FOV = m_PerspectiveFOV;
				params.NearClip = m_PerspectiveNear;
				params.FarClip = m_PerspectiveFar;
				SetProjectionMatrix(params);
				break;
			}
			case ProjectionType::Orthographic:
			{
				params.OrthographicSize = m_OrthographicSize;
				params.NearClip = m_OrthographicNear;
				params.FarClip = m_OrthographicFar;
				SetProjectionMatrix(params);
				break;
			}
		}

		m_IsDirty = false;
	}

	bool Camera::IsPerspective() const
	{
		return m_ProjectionType == ProjectionType::Perspective;
	}

	bool Camera::IsOrthographic() const
	{
		return m_ProjectionType == ProjectionType::Orthographic;
	}

	void Camera::SetPerspective(float FOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = FOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;
		m_IsDirty = true;
	}

	void Camera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;
		m_IsDirty = true;
	}

}
