#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Renderer/Model.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/LightSource2D.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/SceneCamera.h"
#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Editor/EditorResources.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Scene.h"

namespace Vortex {

	void SceneRenderer::RenderScene(const SceneRenderPacket& renderPacket)
	{
		Scene* scene = renderPacket.Scene;
		Camera activeCamera = *renderPacket.MainCamera;
		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

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
				Renderer2D::BeginScene(activeCamera, renderPacket.MainCameraWorldSpaceTransform.GetTransform());
				cameraView = Math::Inverse(renderPacket.MainCameraWorldSpaceTransform.GetTransform());
			}

			// Light Pass 2D
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, LightSource2DComponent>();

				for (const auto e : view)
				{
					auto [transformComponent, lightSource2DComponent] = view.get<TransformComponent, LightSource2DComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::RenderLightSource(transformComponent, lightSource2DComponent);
				}
			}

			// Sprite Pass 2D
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

				for (const auto e : view)
				{
					auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::DrawSprite(
						scene->GetWorldSpaceTransformMatrix(entity),
						spriteRendererComponent,
						(int)(entt::entity)e
					);
				}
			}

			// Circle Pass 2D
			{
				auto group = scene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

				for (const auto e : group)
				{
					auto [transformComponent, circleRendererComponent] = group.get<TransformComponent, CircleRendererComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Renderer2D::DrawCircle(
						scene->GetWorldSpaceTransformMatrix(entity),
						circleRendererComponent.Color,
						circleRendererComponent.Thickness,
						circleRendererComponent.Fade,
						(int)(entt::entity)e
					);
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

						Renderer2D::DrawQuadBillboard(
							cameraView,
							particle.Position,
							size,
							color
						);
					}
				}
			}

			// Render Text
			{
				auto view = scene->GetAllEntitiesWith<TransformComponent, TextMeshComponent>();

				RendererAPI::TriangleCullMode cullMode = Renderer2D::GetCullMode();
				Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::None);

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

				Renderer2D::SetCullMode(cullMode);
			}

			// Scene Icons
			if (renderPacket.EditorScene && projectProps.RendererProps.DisplaySceneIconsInEditor)
			{
				{
					auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();

					for (const auto e : view)
					{
						const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);
						Entity entity{ e, scene };

						if (!entity.IsActive())
							continue;

						const TransformComponent& transform = scene->GetWorldSpaceTransform(entity);

						Renderer2D::DrawQuadBillboard(
							cameraView,
							transform.Translation,
							EditorResources::CameraIcon,
							Math::vec2(projectProps.GizmoProps.GizmoSize),
							ColorToVec4(Color::White),
							(int)(entt::entity)e
						);
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

						const TransformComponent& transform = scene->GetWorldSpaceTransform(entity);

						switch (lightSourceComponent.Type)
						{
							case LightType::Directional:
							{
								Renderer2D::DrawQuadBillboard(
									cameraView,
									transform.Translation,
									EditorResources::SkyLightIcon,
									Math::vec2(projectProps.GizmoProps.GizmoSize),
									ColorToVec4(Color::White),
									(int)(entt::entity)e
								);

								break;
							}
							case LightType::Point:
							{
								Renderer2D::DrawQuadBillboard(
									cameraView,
									transform.Translation,
									EditorResources::PointLightIcon,
									Math::vec2(projectProps.GizmoProps.GizmoSize),
									ColorToVec4(Color::White),
									(int)(entt::entity)e
								);

								break;
							}
							case LightType::Spot:
							{
								Renderer2D::DrawQuadBillboard(
									cameraView,
									transform.Translation,
									EditorResources::SpotLightIcon,
									Math::vec2(projectProps.GizmoProps.GizmoSize),
									ColorToVec4(Color::White),
									(int)(entt::entity)e
								);

								break;
							}
						}
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

						const TransformComponent& transform = scene->GetWorldSpaceTransform(entity);

						Renderer2D::DrawQuadBillboard(
							cameraView,
							transform.Translation,
							EditorResources::AudioSourceIcon,
							Math::vec2(projectProps.GizmoProps.GizmoSize),
							ColorToVec4(Color::White),
							(int)(entt::entity)e
						);
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
				Renderer::BeginScene(editorCamera, renderPacket.TargetFramebuffer);

				SceneRenderer::RenderSkybox(editorCamera->GetViewMatrix(), editorCamera->GetProjectionMatrix(), scene);
			}
			else
			{
				SceneCamera& sceneCamera = reinterpret_cast<SceneCamera&>(activeCamera);
				Renderer::BeginScene(sceneCamera, renderPacket.MainCameraWorldSpaceTransform, renderPacket.TargetFramebuffer);

				Math::mat4 view = Math::Inverse(renderPacket.MainCameraWorldSpaceTransform.GetTransform());
				Math::mat4 projection = sceneCamera.GetProjectionMatrix();

				SceneRenderer::RenderSkybox(view, projection, scene);
			}

			// Light pass
			{
				auto lightSourceView = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

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
			// Sort Meshes by distance from camera and render in reverse order
			std::map<float, Entity> sortedEntities;

			auto meshView = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
			uint32_t i = 0;

			auto SortEntityByDistanceFunc = [&](float distance, Entity entity, uint32_t offset = 0)
			{
				if (sortedEntities.find(distance) == sortedEntities.end())
				{
					sortedEntities[distance] = entity;
					return;
				}

				// slightly modify the distance
				sortedEntities[distance + (0.01f * offset)] = entity;
			};

			for (const auto e : meshView)
			{
				Entity entity{ e, scene };
				Math::vec3 entityWorldSpaceTranslation = scene->GetWorldSpaceTransform(entity).Translation;

				if (!renderPacket.Scene->IsRunning())
				{
					EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
					Math::vec3 cameraPosition = editorCamera->GetPosition();
					float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistanceFunc(distance, entity, i);
				}
				else
				{
					Math::vec3 cameraPosition = renderPacket.MainCameraWorldSpaceTransform.Translation;
					float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistanceFunc(distance, entity, i);
				}

				i++;
			}

			InstrumentationTimer timer("Geometry Pass");
			{
				for (auto it = sortedEntities.crbegin(); it != sortedEntities.crend(); it++)
				{
					Entity entity = it->second;
					MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

					if (!entity.IsActive())
						continue;

					Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

					SharedRef<Model> model = meshRendererComponent.Mesh;
					if (!model)
						continue;

					auto& submeshes = model->GetSubmeshes();

					// render each submesh
					for (auto& submesh : submeshes)
					{
						SharedRef<Material> material = submesh.GetMaterial();

						if (!material)
							continue;

						SetMaterialFlags(material);

						SharedRef<Shader> shader = material->GetShader();
						shader->Enable();

						SceneLightDescription lightDesc = Renderer::GetSceneLightDescription();
						shader->SetBool("u_SceneProperties.HasSkyLight", lightDesc.HasSkyLight);
						shader->SetInt("u_SceneProperties.ActivePointLights", lightDesc.ActivePointLights);
						shader->SetInt("u_SceneProperties.ActiveSpotLights", lightDesc.ActiveSpotLights);
						shader->SetMat4("u_Model", worldSpaceTransform); // should be submesh world transform

						Renderer::BindSkyLightDepthMap();
						Renderer::BindPointLightDepthMaps();
						Renderer::BindSpotLightDepthMaps();

						if (model->HasAnimations() && entity.HasComponent<AnimatorComponent>() && entity.HasComponent<AnimationComponent>())
						{
							shader->SetBool("u_HasAnimations", true);

							const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
							const std::vector<Math::mat4>& transforms = animatorComponent.Animator->GetFinalBoneMatrices();

							for (uint32_t i = 0; i < transforms.size(); i++)
							{
								shader->SetMat4("u_FinalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);
							}
						}
						else
						{
							shader->SetBool("u_HasAnimations", false);
						}

						submesh.Render();

						ResetAllMaterialFlags();
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

	void SceneRenderer::SetMaterialFlags(const SharedRef<Material>& material)
	{
		if (material->HasFlag(MaterialFlag::NoDepthTest))
		{
			RenderCommand::DisableDepthTest();
			RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);
		}
	}

	void SceneRenderer::ResetAllMaterialFlags()
	{
		RendererAPI::TriangleCullMode cullMode = Renderer::GetCullMode();
		RenderCommand::EnableDepthTest();
		RenderCommand::SetCullMode(cullMode);
	}

}
