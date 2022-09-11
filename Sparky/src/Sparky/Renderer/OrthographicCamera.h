#pragma once

#include "Sparky/Core/Core.h"

#include "Sparky/Core/Math.h"

namespace Sparky {

	class SPARKY_API OrthographicCamera
	{
	public:
		OrthographicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		inline const Math::vec3& GetPosition() const { return m_Position; }
		inline void SetPosition(const Math::vec3& position) { m_Position = position; ReCalculateViewMatrix(); }

		inline float GetRotation() const { return m_Rotation; }
		inline void SetRotation(float rotation) { m_Rotation = rotation; ReCalculateViewMatrix(); }

		inline const Math::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		inline const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		inline const Math::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	private:
		void ReCalculateViewMatrix();

	private:
		Math::mat4 m_ProjectionMatrix;
		Math::mat4 m_ViewMatrix;
		Math::mat4 m_ViewProjectionMatrix;

		Math::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		float m_Rotation = 0.0f;
	};

}