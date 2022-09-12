#pragma once

#include "Sparky/Core/Math.h"

namespace Sparky {

	class Camera
	{
	public:
		Camera(const Math::mat4& projection)
			: m_ProjectionMatrix(projection) {}

		const Math::mat4& GetProjection() const { return m_ProjectionMatrix; }

	private:
		Math::mat4 m_ProjectionMatrix;
	};

}
