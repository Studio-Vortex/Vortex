#pragma once

#include <Vortex.h>

namespace Vortex {

	class AssetThumbnailGenerator
	{
	public:
		virtual void OnPrepare(uint32_t renderWidth, uint32_t renderHeight) = 0;
		virtual void OnFinish() = 0;

		virtual SharedReference<Texture2D> GenerateThumbnail(SharedReference<Asset> asset) = 0;

	protected:
		SharedReference<Scene> m_Scene = nullptr;
		SharedReference<Framebuffer> m_TargetFramebuffer = nullptr;

		Math::uvec2 m_RenderSize;

		Actor m_Actor;
		Actor m_DirectionalLight;
		Actor m_CameraActor;
	};

	class SceneAssetThumbnailGenerator : public AssetThumbnailGenerator
	{
	public:
		void OnPrepare(uint32_t renderWidth, uint32_t renderHeight) override;
		void OnFinish() override;

		SharedReference<Texture2D> GenerateThumbnail(SharedReference<Asset> asset) override;
	};

}
