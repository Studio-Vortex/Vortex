#pragma once

#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	class Camera
	{
	public:
		Camera() = default;
		Camera(const Math::mat4& projection, const Math::mat4& unreversedProjection);
		Camera(float FOVdeg, float width, float height, float nearClip, float farClip);
		virtual ~Camera() = default;

		inline const Math::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const Math::mat4& GetUnreversedProjectionMatrix() const { return m_UnreversedProjectionMatrix; }

		void SetProjectionMatrix(const Math::mat4& projection, const Math::mat4& unreversedProjection);
		void SetPerspectiveProjectionMatrix(float FOVrad, float width, float height, float nearClip, float farClip);

	protected:
		Math::mat4 m_ProjectionMatrix = Math::mat4(1.0f);
		Math::mat4 m_UnreversedProjectionMatrix = Math::mat4(1.0f);
	};

}
