#pragma once

#include "Sparky/Core/Math.h"

namespace Sparky {

	class Camera
	{
	public:
		Camera() = default;
		Camera(const Math::mat4& projection)
			: m_ProjectionMatrix(projection) {}
		virtual ~Camera() = default;

		const Math::mat4& GetProjection() const { return m_ProjectionMatrix; }

	protected:
		Math::mat4 m_ProjectionMatrix = Math::mat4(1.0f);
	};

}
