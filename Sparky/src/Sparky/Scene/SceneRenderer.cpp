#include "sppch.h"
#include "SceneRenderer.h"

#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Renderer/ParticleEmitter.h"

#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/EditorCamera.h"

#include "Sparky/Project/Project.h"
#include "Sparky/Scene/Scene.h"

namespace Sparky {

	template <typename TCamera>
	static void RenderScene(TCamera& activeCamera, const TransformComponent& sceneCameraTransform, Scene* scene)
	{
		// Render 2D
		{
			bool sceneCamera = false;
			Math::mat4 cameraView;

			if (std::is_same<TCamera, EditorCamera>())
			{
				EditorCamera& editorCamera = (EditorCamera&)activeCamera;
				Renderer2D::BeginScene(editorCamera);
				cameraView = editorCamera.GetViewMatrix();
			}
			else
			{
				Renderer2D::BeginScene((Camera&)activeCamera, sceneCameraTransform.GetTransform());
				sceneCamera = true;
				cameraView = Math::Inverse(sceneCameraTransform.GetTransform());
			}

			// Render Sprites
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

				for (const auto e : view)
				{
					auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::DrawSprite(scene->GetWorldSpaceTransformMatrix(entity), spriteRendererComponent, (int)(entt::entity)e);
				}
			}

			// Render Circles
			{
				auto group = scene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

				for (const auto e : group)
				{
					auto [transformComponent, circleRendererComponent] = group.get<TransformComponent, CircleRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::DrawCircle(scene->GetWorldSpaceTransformMatrix(entity), circleRendererComponent.Color, circleRendererComponent.Thickness, circleRendererComponent.Fade, (int)(entt::entity)e);
				}
			}

			// Render Particles
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, ParticleEmitterComponent>();

				for (const auto& e : view)
				{
					auto [transformComponent, particleEmitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;

					const auto& particles = particleEmitter->GetParticles();

					// Render the particles in reverse to blend correctly
					for (auto it = particles.crbegin(); it != particles.crend(); it++)
					{
						const ParticleEmitter::Particle& particle = *it;

						if (!particle.Active)
							continue;

						float life = particle.LifeRemaining / particle.LifeTime;
						Math::vec2 size = Math::Lerp(particle.SizeEnd, particle.SizeBegin, life);
						Math::vec4 color = Math::Lerp(particle.ColorEnd, particle.ColorBegin, life);
						Renderer2D::DrawQuadBillboard(cameraView, particle.Position, size, color);
					}
				}
			}

			// Render Text
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, TextMeshComponent>();

				for (auto& e : view)
				{
					auto [transformComponent, textMeshComponent] = view.get<TransformComponent, TextMeshComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::DrawString(
						textMeshComponent.TextString,
						textMeshComponent.FontAsset,
						scene->GetWorldSpaceTransformMatrix(entity),
						textMeshComponent.MaxWidth,
						textMeshComponent.Color,
						textMeshComponent.LineSpacing,
						textMeshComponent.Kerning,
						(int)(entt::entity)e
					);
				}
			}

			// Render Scene Icons
			if (!sceneCamera && Project::GetActive()->GetProperties().RendererProps.DisplaySceneIconsInEditor)
			{
				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();

					for (const auto e : view)
					{
						const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);
						Entity entity{ e, scene };

						if (!entity.IsActive())
							continue;

						Renderer::RenderCameraIcon(scene->GetWorldSpaceTransform(entity), cameraView, (int)(entt::entity)e);
					}
				}

				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

					for (const auto e : view)
					{
						const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(e);
						Entity entity{ e, scene };

						if (!entity.IsActive())
							continue;

						Renderer::RenderLightSourceIcon(scene->GetWorldSpaceTransform(entity), lightSourceComponent, cameraView, (int)(entt::entity)e);
					}
				}

				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, AudioSourceComponent>();

					for (const auto e : view)
					{
						const auto [transformComponent, audioSourceComponent] = view.get<TransformComponent, AudioSourceComponent>(e);
						Entity entity{ e, scene };

						if (!entity.IsActive())
							continue;

						Renderer::RenderAudioSourceIcon(scene->GetWorldSpaceTransform(entity), cameraView, (int)(entt::entity)e);
					}
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D
		{
			if (std::is_same<TCamera, EditorCamera>())
			{
				EditorCamera& editorCamera = reinterpret_cast<EditorCamera&>(activeCamera);
				Renderer::BeginScene(editorCamera);

				SceneRenderer::RenderSkybox(editorCamera.GetViewMatrix(), editorCamera.GetProjection(), scene);
			}
			else
			{
				SceneCamera& sceneCamera = reinterpret_cast<SceneCamera&>(activeCamera);
				Renderer::BeginScene(sceneCamera, sceneCameraTransform);

				Math::mat4 view = Math::Inverse(sceneCameraTransform.GetTransform());
				Math::mat4 projection = sceneCamera.GetProjection();

				SceneRenderer::RenderSkybox(view, projection, scene);
			}

			// TODO: Set Scene Light Description

			// Light pass
			auto lightSourceView = scene->GetAllEntitiesWith<LightSourceComponent>();
			{

				for (auto& e : lightSourceView)
				{
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
					Renderer::RenderLightSource(lightSourceComponent);
				}
			}

			// Geometry pass
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();

				for (const auto e : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer::DrawModel(scene->GetWorldSpaceTransformMatrix(entity), meshRendererComponent);
				}
			}

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderFromSceneCamera(SceneCamera& sceneCamera, const TransformComponent& cameraTransform, Scene* scene)
	{
		RenderScene(sceneCamera, cameraTransform, scene);
	}

	void SceneRenderer::RenderFromEditorCamera(EditorCamera& editorCamera, Scene* scene)
	{
		RenderScene(editorCamera, TransformComponent(), scene);
	}

	void SceneRenderer::RenderSkybox(const Math::mat4& view, const Math::mat4& projection, Scene* scene)
	{
		auto skyboxView = scene->GetAllEntitiesWith<SkyboxComponent>();

		for (auto& e : skyboxView)
		{
			Entity entity{ e, scene };

			if (!entity.IsActive())
				continue;

			SkyboxComponent& skyboxComponent = entity.GetComponent<SkyboxComponent>();
			Renderer::DrawSkybox(view, projection, skyboxComponent.Source);

			// Only render one skybox per scene
			break;
		}
	}

}
