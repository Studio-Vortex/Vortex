#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Renderer/RendererAPI.h"

namespace Vortex {

	struct VORTEX_API ProjectionParams
	{
		f32 Width, Height, NearClip, FarClip;

		float AspectRatio() const { return Width / Height; }
	};

	struct VORTEX_API PerspectiveProjectionParams : public ProjectionParams
	{
		f32 FOV;

		PerspectiveProjectionParams()
		{
			NearClip = 0.01f;
			FarClip = 1000.0f;
		}
	};

	struct VORTEX_API OrthographicProjectionParams : public ProjectionParams
	{
		f32 OrthoSize;

		OrthographicProjectionParams()
		{
			NearClip = -1.0f;
			FarClip = 1.0f;
		}
	};

	class VORTEX_API Camera
	{
	public:
		enum class VORTEX_API ProjectionType { Perspective = 0, Orthographic = 1 };

	public:
		Camera() = default;
		Camera(const PerspectiveProjectionParams& params);
		Camera(const OrthographicProjectionParams& params);
		virtual ~Camera() = default;

		VX_FORCE_INLINE const Math::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		void SetProjectionMatrix(const Math::ProjectionParams& params);

		VX_FORCE_INLINE ProjectionType GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(ProjectionType type);

		VX_FORCE_INLINE const Math::vec2& GetViewportSize() const { return m_ViewportSize; }
		void SetViewportSize(uint32_t width, uint32_t height);

		VX_FORCE_INLINE float GetPerspectiveFOV() const { return m_PerspectiveFOV; }
		VX_FORCE_INLINE void SetPerspectiveFOV(float FOV) { m_PerspectiveFOV = FOV; SetDirty(true); }
		VX_FORCE_INLINE float GetPerspectiveNearClip() const { return m_PerspectiveNear; }
		VX_FORCE_INLINE void SetPerspectiveNearClip(float nearClip) { m_PerspectiveNear = nearClip; SetDirty(true); }
		VX_FORCE_INLINE float GetPerspectiveFarClip() const { return m_PerspectiveFar; }
		VX_FORCE_INLINE void SetPerspectiveFarClip(float farClip) { m_PerspectiveFar = farClip; SetDirty(true); }

		VX_FORCE_INLINE float GetOrthographicSize() const { return m_OrthographicSize; }
		VX_FORCE_INLINE void SetOrthographicSize(float size) { m_OrthographicSize = size; SetDirty(true); }
		VX_FORCE_INLINE float GetOrthographicNearClip() const { return m_OrthographicNear; }
		VX_FORCE_INLINE void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; SetDirty(true); }
		VX_FORCE_INLINE float GetOrthographicFarClip() const { return m_OrthographicFar; }
		VX_FORCE_INLINE void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; SetDirty(true); }

		VX_FORCE_INLINE float GetAspectRatio() const { return m_AspectRatio; }
		VX_FORCE_INLINE void SetAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; SetDirty(true); }

		bool IsPerspective() const;
		bool IsOrthographic() const;

		VX_FORCE_INLINE bool IsDirty() const { return m_IsDirty; }
		VX_FORCE_INLINE void SetDirty(bool dirty) { m_IsDirty = dirty; }

	protected:
		void SetPerspective(float FOV, float nearClip = 0.01f, float farClip = 1000.0f);
		void SetOrthographic(float size, float nearClip = -1.0f, float farClip = 1.0f);

	protected:
		ProjectionType m_ProjectionType = ProjectionType::Perspective;

		Math::mat4 m_ProjectionMatrix = Math::mat4(1.0f);
		Math::vec2 m_ViewportSize = {};

		// Perspective
		float m_PerspectiveFOV = Math::Deg2Rad(45.0f);
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;

		// Orthographic
		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;

		float m_AspectRatio = 0.0f;

		bool m_IsDirty = false;
	};

}
