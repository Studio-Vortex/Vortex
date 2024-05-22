#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Camera.h"

#include "Vortex/Math/Ray.h"

namespace Vortex {

	class VORTEX_API SceneCamera : public Camera
	{
	public:
		SceneCamera() = default;
		~SceneCamera() override = default;

		Math::Ray Raycast(const Math::vec2& point, const Math::vec3& cameraPosition, float maxDistance, const Math::mat4& view) const;
		Math::vec3 ScreenPointToWorldPoint(const Math::vec2& point, const Math::vec2& viewportMinBound, const Math::vec3& cameraPosition, const float maxDistance, const Math::mat4& view) const;
		Math::vec2 ScreenPointToViewportPoint(const Math::vec2& point) const;
	};

}
