#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"

#include "Sparky/Renderer/ParticleEmitter.h"

namespace Sparky {

	template <typename TCamera>
	static void RenderScene(TCamera& activeCamera, const TransformComponent& sceneCameraTransform, entt::registry& sceneRegistry)
	{
		// Render 2D
		{
			bool sceneCamera = false;
			Math::vec3 cameraPosition;

			if (typeid(TCamera).name() == typeid(EditorCamera).name())
			{
				EditorCamera& editorCamera = reinterpret_cast<EditorCamera&>(activeCamera);
				Renderer2D::BeginScene(editorCamera);
				cameraPosition = editorCamera.GetPosition();
			}
			else
			{
				Renderer2D::BeginScene(reinterpret_cast<Camera&>(activeCamera), sceneCameraTransform.GetTransform());
				sceneCamera = true;
				cameraPosition = sceneCameraTransform.Translation;
			}

			// Render Sprites
			{
				auto view = sceneRegistry.view<TransformComponent, SpriteRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteRendererComponent, (int)(entt::entity)entity);
				}
			}

			// Render Circles
			{
				auto group = sceneRegistry.group<TransformComponent>(entt::get<CircleRendererComponent>);

				for (const auto entity : group)
				{
					auto [transformComponent, circleRendererComponent] = group.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circleRendererComponent.Color, circleRendererComponent.Thickness, circleRendererComponent.Fade, (int)(entt::entity)entity);
				}
			}

			// Render Particles
			{
				auto view = sceneRegistry.view<TransformComponent, ParticleEmitterComponent>();

				for (const auto& entity : view)
				{
					auto [transformComponent, particleEmitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(entity);

					SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;
					
					if (!particleEmitter->IsActive())
						continue;

					const auto& particles = particleEmitter->GetParticles();

					for (auto& particle : particles)
					{
						if (!particle.Active)
							continue;

						float life = particle.LifeRemaining / particle.LifeTime;
						Math::vec2 size = Math::Lerp(particle.SizeEnd, particle.SizeBegin, life);
						Math::vec4 color = Math::Lerp(particle.ColorEnd, particle.ColorBegin, life);
						Renderer2D::DrawRotatedQuad(particle.Position, size, particle.Rotation, color, (int)(entt::entity)entity);
					}
				}
			}

			// Render Scene Icons
			if (!sceneCamera)
			{
				{
					auto view = sceneRegistry.view<TransformComponent, CameraComponent>();

					for (const auto entity : view)
					{
						const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(entity);

						Renderer::RenderCameraIcon(transformComponent, cameraPosition, (int)(entt::entity)entity);
					}
				}

				{
					auto view = sceneRegistry.view<TransformComponent, LightSourceComponent>();

					for (const auto entity : view)
					{
						const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(entity);

						Renderer::RenderLightSourceIcon(transformComponent, cameraPosition, (int)(entt::entity)entity);
					}
				}

				{
					auto view = sceneRegistry.view<TransformComponent, AudioSourceComponent>();

					for (const auto entity : view)
					{
						const auto [transformComponent, audioSourceComponent] = view.get<TransformComponent, AudioSourceComponent>(entity);

						Renderer::RenderAudioSourceIcon(transformComponent, cameraPosition, (int)(entt::entity)entity);
					}
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D
		{
			if (typeid(TCamera).name() == typeid(EditorCamera).name())
			{
				EditorCamera& editorCamera = reinterpret_cast<EditorCamera&>(activeCamera);
				Renderer::BeginScene(editorCamera);

				SceneRenderer::RenderSkybox(editorCamera.GetViewMatrix(), editorCamera.GetProjection(), sceneRegistry);
			}
			else
			{
				SceneCamera& sceneCamera = reinterpret_cast<SceneCamera&>(activeCamera);
				Renderer::BeginScene(sceneCamera, sceneCameraTransform);

				Math::mat4 view = Math::Inverse(sceneCameraTransform.GetTransform());
				Math::mat4 projection = sceneCamera.GetProjection();

				SceneRenderer::RenderSkybox(view, projection, sceneRegistry);
			}

			// Render Light Sources
			{
				auto view = sceneRegistry.view<LightSourceComponent>();

				for (auto& entity : view)
				{
					const auto& lightSourceComponent = view.get<LightSourceComponent>(entity);

					Renderer::RenderLightSource(lightSourceComponent);
				}
			}

			// Render Meshes
			{
				auto view = sceneRegistry.view<TransformComponent, MeshRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Mesh)
						Renderer::DrawModel(transformComponent, meshRendererComponent, (int)(entt::entity)entity);
				}
			}

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderFromSceneCamera(SceneCamera& sceneCamera, const TransformComponent& cameraTransform, entt::registry& sceneRegistry)
	{
		RenderScene(sceneCamera, cameraTransform, sceneRegistry);
	}

	void SceneRenderer::RenderFromEditorCamera(EditorCamera& editorCamera, entt::registry& sceneRegistry)
	{
		RenderScene(editorCamera, TransformComponent(), sceneRegistry);
	}

	void SceneRenderer::RenderSkybox(const Math::mat4& view, const Math::mat4& projection, entt::registry& sceneRegistry)
	{
		auto skyboxView = sceneRegistry.view<SkyboxComponent>();

		if (!skyboxView.empty())
		{
			for (auto& entity : skyboxView)
			{
				auto& skyboxComponent = skyboxView.get<SkyboxComponent>(entity);
				Renderer::DrawSkybox(view, projection, skyboxComponent.Source);

				// Only render one skybox per scene
				break;
			}
		}
	}

}
