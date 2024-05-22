#include "AssetThumbnailGenerator.h"

namespace Vortex {

	namespace Utils {

		SharedReference<Framebuffer> SetupDefaultFramebuffer(const Math::uvec2& renderSize)
		{
			FramebufferProperties properties;
			properties.Attachments = {
				ImageFormat::RGBA16F,
				ImageFormat::RED_INTEGER,
				ImageFormat::RGBA16F,
				ImageFormat::Depth,
			};
			properties.Width = renderSize.x;
			properties.Height = renderSize.y;
			properties.Samples = 1;
			properties.SwapChainTarget = false;
			return Framebuffer::Create(properties);
		}

		Actor SetupDefaultDirLight(SharedReference<Scene> scene)
		{
			Actor actor = scene->CreateActor("Directional Light");
			actor.GetTransform().Translation = Math::vec3(-5, 5, -5);
			actor.GetTransform().SetRotationEuler(Math::vec3(Math::Deg2Rad(1.0f), Math::Deg2Rad(-1.0f), Math::Deg2Rad(1.0f)));
			LightSourceComponent& lightSourceComponent = actor.AddComponent<LightSourceComponent>();
			lightSourceComponent.Type = LightType::Directional;
			lightSourceComponent.ShadowBias = 0.0f;
			return actor;
		}

		Actor SetupDefaultCamera(SharedReference<Scene> scene)
		{
			Actor actor = scene->CreateActor("Camera");
			actor.GetTransform().Translation = Math::vec3(0, 0, 5);
			CameraComponent& cameraComponent = actor.AddComponent<CameraComponent>();
			cameraComponent.Primary = true;
			return actor;
		}

	}


	void SceneAssetThumbnailGenerator::OnPrepare(uint32_t renderWidth, uint32_t renderHeight)
	{
		m_RenderSize = Math::uvec2(renderWidth, renderHeight);
		m_TargetFramebuffer = Utils::SetupDefaultFramebuffer(m_RenderSize);
		m_Scene = Scene::Create(m_TargetFramebuffer);

		m_Actor = m_Scene->CreateActor("Actor");
		m_DirectionalLight = Utils::SetupDefaultDirLight(m_Scene);
		m_CameraActor = Utils::SetupDefaultCamera(m_Scene);
	}

	void SceneAssetThumbnailGenerator::OnFinish()
	{
		
	}

	SharedReference<Texture2D> SceneAssetThumbnailGenerator::GenerateThumbnail(SharedReference<Asset> asset)
	{
		return SharedReference<Texture2D>();
	}

}
