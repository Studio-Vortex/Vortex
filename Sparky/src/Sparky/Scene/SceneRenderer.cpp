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

			if (typeid(TCamera).name() == typeid(EditorCamera).name())
			{
				EditorCamera& editorCamera = reinterpret_cast<EditorCamera&>(activeCamera);
				Renderer2D::BeginScene(editorCamera);
				cameraView = Math::Inverse(TransformComponent{
					editorCamera.GetPosition(), { -editorCamera.GetPitch(), -editorCamera.GetYaw(), 0.0f }, { 1, 1, 1 }
				}.GetTransform());
			}
			else
			{
				Renderer2D::BeginScene(reinterpret_cast<Camera&>(activeCamera), sceneCameraTransform.GetTransform());
				sceneCamera = true;
				cameraView = Math::Inverse(sceneCameraTransform.GetTransform());
			}

			// Render Sprites
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(scene->GetWorldSpaceTransformMatrix(Entity{ entity, scene }), spriteRendererComponent, (int)(entt::entity)entity);
				}
			}

			// Render Circles
			{
				auto group = scene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

				for (const auto entity : group)
				{
					auto [transformComponent, circleRendererComponent] = group.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(scene->GetWorldSpaceTransformMatrix(Entity{ entity, scene }), circleRendererComponent.Color, circleRendererComponent.Thickness, circleRendererComponent.Fade, (int)(entt::entity)entity);
				}
			}

			// Render Particles
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, ParticleEmitterComponent>();

				for (const auto& entity : view)
				{
					auto [transformComponent, particleEmitterComponent] = view.get<TransformComponent, ParticleEmitterComponent>(entity);

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

					for (const auto entity : view)
					{
						const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(entity);

						Renderer::RenderCameraIcon(scene->GetWorldSpaceTransform(Entity{ entity, scene }), cameraView, (int)(entt::entity)entity);
					}
				}

				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

					for (const auto entity : view)
					{
						const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(entity);

						Renderer::RenderLightSourceIcon(scene->GetWorldSpaceTransform(Entity{ entity, scene }), lightSourceComponent, cameraView, (int)(entt::entity)entity);
					}
				}

				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, AudioSourceComponent>();

					for (const auto entity : view)
					{
						const auto [transformComponent, audioSourceComponent] = view.get<TransformComponent, AudioSourceComponent>(entity);

						Renderer::RenderAudioSourceIcon(scene->GetWorldSpaceTransform(Entity{ entity, scene }), cameraView, (int)(entt::entity)entity);
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

			// Light pass
			{
				auto view = scene->GetAllEntitiesWith<LightSourceComponent>();

				for (auto& entity : view)
				{
					const auto& lightSourceComponent = view.get<LightSourceComponent>(entity);

					Renderer::RenderLightSource(lightSourceComponent);
				}
			}

			// Geometry pass
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();

				for (const auto entity : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(entity);

					if (meshRendererComponent.Mesh)
						Renderer::DrawModel(scene->GetWorldSpaceTransformMatrix(Entity{ entity, scene }), meshRendererComponent);
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
