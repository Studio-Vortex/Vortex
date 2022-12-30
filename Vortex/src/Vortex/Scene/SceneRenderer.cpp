#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Renderer/Model.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/ParticleEmitter.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/SceneCamera.h"
#include "Vortex/Renderer/EditorCamera.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Scene.h"

namespace Vortex {

	void SceneRenderer::RenderScene(const SceneRenderPacket& renderPacket)
	{
		Scene* scene = renderPacket.Scene;
		Camera activeCamera = *renderPacket.MainCamera;

		// Render 2D
		{
			Math::mat4 cameraView;

			if (renderPacket.EditorScene)
			{
				EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
				Renderer2D::BeginScene(editorCamera);
				cameraView = editorCamera->GetViewMatrix();
			}
			else
			{
				Renderer2D::BeginScene((Camera&)activeCamera, renderPacket.CameraWorldSpaceTransform.GetTransform());
				cameraView = Math::Inverse(renderPacket.CameraWorldSpaceTransform.GetTransform());
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
			if (renderPacket.EditorScene && Project::GetActive()->GetProperties().RendererProps.DisplaySceneIconsInEditor)
			{
				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();

					for (const auto e : view)
					{
						const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);
						Entity entity{ e, scene };

						if (!entity.IsActive())
							continue;

						Renderer::RenderCameraIcon(scene->GetWorldSpaceTransform(entity), cameraView);
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

						Renderer::RenderLightSourceIcon(scene->GetWorldSpaceTransform(entity), lightSourceComponent, cameraView);
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

						Renderer::RenderAudioSourceIcon(scene->GetWorldSpaceTransform(entity), cameraView);
					}
				}
			}

			Renderer2D::EndScene();
		}

		// Render 3D
		{
			if (renderPacket.EditorScene)
			{
				EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
				Renderer::BeginScene(editorCamera);

				SceneRenderer::RenderSkybox(editorCamera->GetViewMatrix(), editorCamera->GetProjection(), scene);
			}
			else
			{
				SceneCamera& sceneCamera = reinterpret_cast<SceneCamera&>(activeCamera);
				Renderer::BeginScene(sceneCamera, renderPacket.CameraWorldSpaceTransform);

				Math::mat4 view = Math::Inverse(renderPacket.CameraWorldSpaceTransform.GetTransform());
				Math::mat4 projection = sceneCamera.GetProjection();

				SceneRenderer::RenderSkybox(view, projection, scene);
			}

			// Light pass
			auto lightSourceView = scene->GetAllEntitiesWith<LightSourceComponent>();
			{
				for (auto& e : lightSourceView)
				{
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
					Renderer::RenderLightSource(scene->GetWorldSpaceTransform(entity), lightSourceComponent);
				}
			}

			// Geometry pass
			InstrumentationTimer timer("Geometry Pass");
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();

				for (const auto e : view)
				{
					auto [transformComponent, meshRendererComponent] = view.get<TransformComponent, MeshRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer::BindSkyLightDepthMap();
					Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

					if (entity.HasComponent<AnimatorComponent>() && entity.HasComponent<AnimationComponent>() && meshRendererComponent.Mesh->HasAnimations())
					{
						meshRendererComponent.Mesh->Render(worldSpaceTransform, entity.GetComponent<AnimatorComponent>());
					}
					else
					{
						meshRendererComponent.Mesh->Render(worldSpaceTransform);
					}
				}
			}

			RenderTime& renderTime = Renderer::GetRenderTime();
			renderTime.GeometryPassRenderTime += timer.ElapsedMS();

			Renderer::EndScene();
		}
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
			Renderer::DrawEnvironmentMap(view, projection, skyboxComponent);

			// Only render one skybox per scene
			break;
		}
	}

}
