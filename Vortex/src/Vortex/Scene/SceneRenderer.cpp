#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/LightSource2D.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/SceneCamera.h"
#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Editor/EditorResources.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Scene.h"

namespace Vortex {

	static AssetHandle s_EnvironmentHandle = 0;

	void SceneRenderer::RenderScene(const SceneRenderPacket& renderPacket)
	{
		Scene* scene = renderPacket.Scene;
		Camera activeCamera = *renderPacket.MainCamera;
		SharedReference<Project> activeProject = Project::GetActive();
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

					AssetHandle textureHandle = spriteRendererComponent.Texture;
					SharedReference<Texture2D> texture = nullptr;

					if (AssetManager::IsHandleValid(textureHandle))
						texture = AssetManager::GetAsset<Texture2D>(textureHandle);

					Renderer2D::DrawSprite(
						scene->GetWorldSpaceTransformMatrix(entity),
						spriteRendererComponent,
						texture,
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

				for (const auto e : view)
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

						const float life = particle.LifeRemaining / particle.LifeTime;
						Math::vec2 size = Math::Lerp(particle.SizeEnd, particle.SizeBegin, life);
						Math::vec4 color;

						if (particleEmitter->GetProperties().GenerateRandomColors)
							color = particle.RandomColor;
						else
							color = Math::Lerp(particle.ColorEnd, particle.ColorBegin, life);

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

				for (const auto e : view)
				{
					auto [transformComponent, textMeshComponent] = view.get<TransformComponent, TextMeshComponent>(e);
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					AssetHandle fontAssetHandle = textMeshComponent.FontAsset;
					if (!AssetManager::IsHandleValid(fontAssetHandle))
						continue;

					SharedReference<Font> font = AssetManager::GetAsset<Font>(fontAssetHandle);
					if (!font)
						continue;

					Renderer2D::DrawString(
						textMeshComponent.TextString,
						font,
						scene->GetWorldSpaceTransformMatrix(entity),
						textMeshComponent.MaxWidth,
						textMeshComponent.Color,
						textMeshComponent.BgColor,
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

						SharedReference<Texture2D> icon = nullptr;

						if (lightSourceComponent.Type == LightType::Directional)
							icon = EditorResources::SkyLightIcon;
						if (lightSourceComponent.Type == LightType::Point)
							icon = EditorResources::PointLightIcon;
						if (lightSourceComponent.Type == LightType::Spot)
							icon = EditorResources::SpotLightIcon;

						Renderer2D::DrawQuadBillboard(
							cameraView,
							transform.Translation,
							icon,
							Math::vec2(projectProps.GizmoProps.GizmoSize),
							ColorToVec4(Color::White),
							(int)(entt::entity)e
						);
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
			Math::mat4* view = nullptr;
			Math::mat4* projection = nullptr;
			SkyboxComponent skyboxComponent;
			SharedReference<Skybox> environment = nullptr;

			auto skyboxView = scene->GetAllEntitiesWith<SkyboxComponent>();

			// Only render one environment per scene
			for (const auto e : skyboxView)
			{
				Entity entity{ e, scene };

				if (!entity.IsActive())
					continue;

				skyboxComponent = entity.GetComponent<SkyboxComponent>();
				AssetHandle environmentHandle = skyboxComponent.Skybox;
				if (!AssetManager::IsHandleValid(environmentHandle))
					continue;

				environment = AssetManager::GetAsset<Skybox>(environmentHandle);
				if (!environment)
					continue;

				if (environmentHandle != s_EnvironmentHandle)
				{
					s_EnvironmentHandle = environmentHandle;
					Renderer::CreateEnvironmentMap(skyboxComponent, environment);
					Renderer::SetEnvironment(environment);
				}

				break;
			}

			if (renderPacket.EditorScene)
			{
				EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
				Renderer::BeginScene(editorCamera, renderPacket.TargetFramebuffer);

				const auto& editorCameraView = editorCamera->GetViewMatrix();
				const auto& editorCameraProjection = editorCamera->GetProjectionMatrix();
				view = (Math::mat4*)&editorCameraView;
				projection = (Math::mat4*)&editorCameraProjection;
			}
			else
			{
				SceneCamera& sceneCamera = reinterpret_cast<SceneCamera&>(activeCamera);
				Renderer::BeginScene(sceneCamera, renderPacket.MainCameraWorldSpaceTransform, renderPacket.TargetFramebuffer);

				auto sceneCameraView = Math::Inverse(renderPacket.MainCameraWorldSpaceTransform.GetTransform());
				const auto& sceneCameraProjection = sceneCamera.GetProjectionMatrix();

				view = &sceneCameraView;
				projection = (Math::mat4*)&sceneCameraProjection;
			}

			const bool hasEnvironment = view && projection && environment;

			if (s_EnvironmentHandle != 0 && hasEnvironment)
			{
				Renderer::DrawEnvironmentMap(*view, *projection, skyboxComponent, environment);
			}
			else
			{
				environment = nullptr;
				Renderer::SetEnvironment(environment);
			}

			// Light pass
			{
				auto lightSourceView = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

				for (const auto e : lightSourceView)
				{
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
					Renderer::RenderLightSource(scene->GetWorldSpaceTransform(entity), lightSourceComponent);
				}
			}

			// Geometry pass
			// Sort All Meshes by distance from camera and render in reverse order
			std::map<float, Entity> sortedEntities;

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

			// Sort Meshes
			{
				auto meshRendererView = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
				uint32_t i = 0;

				for (const auto e : meshRendererView)
				{
					Entity entity{ e, scene };
					if (!entity.IsActive())
						continue;

					Math::vec3 entityWorldSpaceTranslation = scene->GetWorldSpaceTransform(entity).Translation;

					if (renderPacket.EditorScene)
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
			}

			// Sort Static Meshes
			{
				auto staticMeshRendererView = scene->GetAllEntitiesWith<TransformComponent, StaticMeshRendererComponent>();
				uint32_t i = 0;

				for (const auto e : staticMeshRendererView)
				{
					Entity entity{ e, scene };

					if (!entity.IsActive())
						continue;

					Math::vec3 entityWorldSpaceTranslation = scene->GetWorldSpaceTransform(entity).Translation;

					if (renderPacket.EditorScene)
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
			}

			InstrumentationTimer timer("Geometry Pass");
			{
				SceneLightDescription sceneLightDesc = Renderer::GetSceneLightDescription();

				for (auto it = sortedEntities.crbegin(); it != sortedEntities.crend(); it++)
				{
					Entity entity = it->second;

					if (entity.HasComponent<MeshRendererComponent>())
					{
						const MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

						Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

						AssetHandle meshHandle = meshRendererComponent.Mesh;
						if (!AssetManager::IsHandleValid(meshHandle))
							continue;

						SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
						if (!mesh)
							continue;

						auto& submesh = mesh->GetSubmesh();

						SharedReference<Material> material = submesh.GetMaterial();
						if (!material)
							continue;

						SetMaterialFlags(material);

						SharedReference<Shader> shader = material->GetShader();
						shader->Enable();

						shader->SetBool("u_SceneProperties.HasSkyLight", sceneLightDesc.HasSkyLight);
						shader->SetInt("u_SceneProperties.ActivePointLights", sceneLightDesc.ActivePointLights);
						shader->SetInt("u_SceneProperties.ActiveSpotLights", sceneLightDesc.ActiveSpotLights);
						shader->SetMat4("u_Model", worldSpaceTransform); // should be submesh world transform

						Renderer::BindSkyLightDepthMap();
						Renderer::BindPointLightDepthMaps();
						Renderer::BindSpotLightDepthMaps();

						if (mesh->HasAnimations() && entity.HasComponent<AnimatorComponent>() && entity.HasComponent<AnimationComponent>())
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
					else if (entity.HasComponent<StaticMeshRendererComponent>())
					{
						const StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

						Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

						AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
						if (!AssetManager::IsHandleValid(staticMeshHandle))
							continue;

						SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
						if (!staticMesh)
							continue;

						auto& submeshes = staticMesh->GetSubmeshes();

						// render each submesh
						for (auto& submesh : submeshes)
						{
							if (!AssetManager::IsHandleValid(submesh.GetMaterial()))
								continue;

							SharedReference<Material> material = AssetManager::GetAsset<Material>(submesh.GetMaterial());
							if (!material)
								continue;

							SetMaterialFlags(material);

							SharedReference<Shader> shader = material->GetShader();
							shader->Enable();

							shader->SetBool("u_SceneProperties.HasSkyLight", sceneLightDesc.HasSkyLight);
							shader->SetInt("u_SceneProperties.ActivePointLights", sceneLightDesc.ActivePointLights);
							shader->SetInt("u_SceneProperties.ActiveSpotLights", sceneLightDesc.ActiveSpotLights);
							shader->SetMat4("u_Model", worldSpaceTransform); // should be submesh world transform

							Renderer::BindSkyLightDepthMap();
							Renderer::BindPointLightDepthMaps();
							Renderer::BindSpotLightDepthMaps();

							submesh.Render();

							ResetAllMaterialFlags();
						}
					}
				}
			}

			RenderTime& renderTime = Renderer::GetRenderTime();
			renderTime.GeometryPassRenderTime += timer.ElapsedMS();

			Renderer::EndScene();
		}
	}

	void SceneRenderer::RenderEnvironment(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment)
	{
		Renderer::DrawEnvironmentMap(view, projection, skyboxComponent, environment);
	}

	void SceneRenderer::SetMaterialFlags(const SharedReference<Material>& material)
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
