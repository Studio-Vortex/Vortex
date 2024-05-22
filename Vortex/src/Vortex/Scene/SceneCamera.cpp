#include "vxpch.h"
#include "SceneCamera.h"

namespace Vortex {

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
