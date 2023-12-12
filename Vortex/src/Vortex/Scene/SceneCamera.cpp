#include "vxpch.h"
#include "SceneCamera.h"

#include "Vortex/Editor/UI/UI.h"

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
			{
				SetPerspectiveProjectionMatrix(m_PerspectiveFOV, (float)width, (float)height, m_PerspectiveNear, m_PerspectiveFar);
				break;
			}
			case ProjectionType::Orthographic:
			{
				float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
				float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
				float orthoBottom = -m_OrthographicSize * 0.5f;
				float orthoTop = m_OrthographicSize * 0.5f;

				m_ProjectionMatrix = Math::Ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
				break;
			}
		}
	}

	void SceneCamera::SetProjectionType(ProjectionType type)
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

		const uint32_t width = (uint32_t)m_ViewportSize.x;
		const uint32_t height = (uint32_t)m_ViewportSize.y;

		if (width != 0 && height != 0)
		{
			SetViewportSize(width, height);
		}
	}

	Math::Ray SceneCamera::Raycast(const Math::vec2& point, const Math::vec3& cameraPosition, float maxDistance, const Math::mat4& view) const
	{
		const Math::vec4 mouseClipPos = { point.x, point.y, -1.0f, 1.0f };

		const auto inverseProj = Math::Inverse(m_ProjectionMatrix);
		const auto inverseView = Math::Inverse(Math::mat3(view));

		const Math::vec4 ray = inverseProj * mouseClipPos;
		const Math::vec3 rayPos = cameraPosition;
		const Math::vec3 rayDir = inverseView * Math::vec3(ray);
		
		return Math::Ray(rayPos, rayDir);
	}

	Math::vec3 SceneCamera::ScreenPointToWorldPoint(const Math::vec2& point, const Math::vec2& viewportMinBound, const Math::vec3& cameraPosition, const float maxDistance, const Math::mat4& view) const
	{
		const Math::vec2 ndc = ScreenPointToViewportPoint(point - viewportMinBound);

		const Math::vec4 clipSpace = Math::vec4(ndc.x, ndc.y, -1.0f, 1.0f);

		const Math::mat4 inverseViewProj = Math::Inverse(m_ProjectionMatrix * view);
		const Math::vec4 worldPoint = inverseViewProj * clipSpace;

		const Math::vec3 direction = Math::Normalize(Math::vec3(worldPoint) - cameraPosition);

		return cameraPosition + (direction * maxDistance);
	}

	Math::vec2 SceneCamera::ScreenPointToViewportPoint(const Math::vec2& point) const
	{
		return Math::vec2{
			(point.x / m_ViewportSize.x) * 2.0f - 1.0f,
			((point.y / m_ViewportSize.y) * 2.0f - 1.0f)
		};
	}

}
