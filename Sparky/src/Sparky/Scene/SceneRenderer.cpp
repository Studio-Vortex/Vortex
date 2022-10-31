#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"

#include "Sparky/Renderer/ParticleEmitter.h"

namespace Sparky {

	template <typename TCamera>
	static void RenderScene(TCamera& activeCamera, const Math::mat4& sceneCameraTransform, entt::registry& sceneRegistry)
	{
		// Render 2D
		{
			if (typeid(TCamera).name() == typeid(EditorCamera).name())
				Renderer2D::BeginScene(reinterpret_cast<EditorCamera&>(activeCamera));
			else
				Renderer2D::BeginScene(reinterpret_cast<Camera&>(activeCamera), sceneCameraTransform);

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

				for (auto& entity : view)
				{
					auto [transformComponent, particleEmitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(entity);

					SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;
					
					if (particleEmitter->IsActive())
					{
						std::vector<ParticleEmitter::Particle> particles = particleEmitter->GetParticles();
						const ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();
						for (auto& particle : particles)
						{
							switch (emitterProperties.Type)
							{
								case ParticleEmitterProperties::PrimitiveType::Quad:
									Renderer2D::DrawQuad(particle.Position, particle.Size, particle.Color, (int)(entt::entity)entity);
									break;
								case ParticleEmitterProperties::PrimitiveType::Circle:
									Math::mat4 transform = Math::Identity() * Math::Translate(particle.Position) * Math::Scale({ particle.Size.x, particle.Size.y, 1.0f });
									Renderer2D::DrawCircle(transform, particle.Color, 1.0f, 0.005f, (int)(entt::entity)entity);
									break;
							}
						}
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

				Math::mat4 view = Math::Inverse(sceneCameraTransform);
				Math::mat4 projection = sceneCamera.GetProjection();

				SceneRenderer::RenderSkybox(view, projection, sceneRegistry);
			}

			// Render Lights
			{
				auto view = sceneRegistry.view<TransformComponent, LightSourceComponent>();

				if (typeid(TCamera).name() == typeid(EditorCamera).name())
					Renderer2D::BeginScene(reinterpret_cast<EditorCamera&>(activeCamera));
				else
					Renderer2D::BeginScene(reinterpret_cast<Camera&>(activeCamera), sceneCameraTransform);
				
				for (const auto entity : view)
				{
					auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(entity);

					Renderer::RenderLightSource(transformComponent, lightSourceComponent, (int)(entt::entity)entity);
				}

				Renderer2D::EndScene();
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

	void SceneRenderer::RenderFromSceneCamera(SceneCamera* sceneCamera, const Math::mat4& cameraTransform, entt::registry& sceneRegistry)
	{
		RenderScene(*sceneCamera, cameraTransform, sceneRegistry);
	}

	void SceneRenderer::RenderFromEditorCamera(EditorCamera& editorCamera, entt::registry& sceneRegistry)
	{
		RenderScene(editorCamera, Math::Identity(), sceneRegistry);
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
