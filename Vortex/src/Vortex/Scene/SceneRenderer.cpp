#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Asset/AssetManager.h"
#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/LightSource2D.h"

#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Editor/EditorResources.h"

namespace Vortex {

	static AssetHandle s_EnvironmentHandle = 0;
	static SharedReference<Skybox> s_EmptyEnvironment = nullptr;

	void SceneRenderer::RenderScene(const SceneRenderPacket& renderPacket)
	{
		VX_CORE_ASSERT(renderPacket.Scene, "Invalid Scene!");

		OnRenderScene2D(renderPacket);
		OnRenderScene3D(renderPacket);
	}

	void SceneRenderer::BeginSceneRenderer2D(const SceneRenderPacket& renderPacket)
	{
		if (renderPacket.EditorScene)
		{
			Renderer2D::BeginScene((EditorCamera*)renderPacket.MainCamera);
		}
		else
		{
			Renderer2D::BeginScene((SceneCamera&)*renderPacket.MainCamera, renderPacket.MainCameraViewMatrix);
		}
	}

	void SceneRenderer::EndSceneRenderer2D()
	{
		Renderer2D::EndScene();
	}

	void SceneRenderer::BeginSceneRenderer(const SceneRenderPacket& renderPacket)
	{
		if (renderPacket.EditorScene)
		{
			Renderer::BeginScene((EditorCamera*)renderPacket.MainCamera, renderPacket.TargetFramebuffer);
		}
		else
		{
			Renderer::BeginScene((SceneCamera&)*renderPacket.MainCamera, renderPacket.MainCameraViewMatrix, renderPacket.MainCameraWorldSpaceTranslation, renderPacket.TargetFramebuffer);
		}
	}

	void SceneRenderer::EndSceneRenderer()
	{
		Renderer::EndScene();
	}

	void SceneRenderer::OnRenderScene2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Camera activeCamera = *renderPacket.MainCamera;
		Math::mat4 cameraView = renderPacket.MainCameraViewMatrix;

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		BeginSceneRenderer2D(renderPacket);

		LightPass2D(renderPacket);

		SpritePass(renderPacket);

		ParticlePass(renderPacket);

		TextPass(renderPacket);

		// Scene Icons
		if (renderPacket.EditorScene && projectProps.RendererProps.DisplaySceneIconsInEditor)
		{
			SceneIconPass(renderPacket);
		}

		EndSceneRenderer2D();
	}

	void SceneRenderer::OnRenderScene3D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4* view = (Math::mat4*)&renderPacket.MainCameraViewMatrix;
		Math::mat4* projection = (Math::mat4*)&renderPacket.MainCameraProjectionMatrix;

		SkyboxComponent skyboxComponent;
		SharedReference<Skybox> environment = nullptr;

		FindCurrentEnvironment(renderPacket, skyboxComponent, environment);

		BeginSceneRenderer(renderPacket);

		const bool hasEnvironment = s_EnvironmentHandle != 0 && view && projection && environment;

		if (hasEnvironment)
		{
			RenderEnvironment(*view, *projection, &skyboxComponent, environment);
		}
		else
		{
			ClearEnvironment();
		}

		LightPass3D(renderPacket);

		const std::map<float, Entity>& sortedEntities = SortMeshGeometry(renderPacket);

		GeometryPass(renderPacket, sortedEntities);

		EndSceneRenderer();
	}

	void SceneRenderer::LightPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

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

	void SceneRenderer::SpritePass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

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
	}

	void SceneRenderer::ParticlePass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;
		Math::mat4 cameraView = renderPacket.MainCameraViewMatrix;

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

	void SceneRenderer::TextPass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto view = scene->GetAllEntitiesWith<TransformComponent, TextMeshComponent>();

		RendererAPI::TriangleCullMode originalCullMode = Renderer2D::GetCullMode();
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

			Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

			Renderer2D::DrawString(
				textMeshComponent.TextString,
				font,
				worldSpaceTransform,
				textMeshComponent.MaxWidth,
				textMeshComponent.Color,
				textMeshComponent.BgColor,
				textMeshComponent.LineSpacing,
				textMeshComponent.Kerning,
				(int)(entt::entity)e
			);
		}

		Renderer2D::SetCullMode(originalCullMode);
	}

	void SceneRenderer::SceneIconPass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(renderPacket.EditorScene, "Scene Icons can only be rendered in a Editor Scene!");

		Scene* scene = renderPacket.Scene;
		Math::mat4 cameraView = renderPacket.MainCameraViewMatrix;

		const ProjectProperties& projectProps = Project::GetActive()->GetProperties();

		// Camera Icons
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

		// Light Icons
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

			for (const auto e : view)
			{
				const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(e);
				Entity entity{ e, scene };

				if (!entity.IsActive())
					continue;

				const TransformComponent& transform = scene->GetWorldSpaceTransform(entity);

				static const SharedReference<Texture2D> icons[3] =
				{
					EditorResources::SkyLightIcon,
					EditorResources::PointLightIcon,
					EditorResources::SpotLightIcon,
				};

				Renderer2D::DrawQuadBillboard(
					cameraView,
					transform.Translation,
					icons[(uint32_t)lightSourceComponent.Type],
					Math::vec2(projectProps.GizmoProps.GizmoSize),
					ColorToVec4(Color::White),
					(int)(entt::entity)e
				);
			}
		}

		// Audio Icons
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

	void SceneRenderer::FindCurrentEnvironment(const SceneRenderPacket& renderPacket, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment)
	{
		Scene* scene = renderPacket.Scene;

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
				SetEnvironment(environmentHandle, skyboxComponent, environment);
			}

			return;
		}
	}

	void SceneRenderer::LightPass3D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

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

	std::map<float, Entity> SceneRenderer::SortMeshGeometry(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		InstrumentationTimer timer("Pre-Geo-Pass Sort");
		Scene* scene = renderPacket.Scene;

		// Sort All Meshes by distance from camera
		std::map<float, Entity> sortedEntities;

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

					SortEntityByDistance(sortedEntities, distance, entity, i);
				}
				else
				{
					Math::vec3 cameraPosition = renderPacket.MainCameraWorldSpaceTranslation;
					float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedEntities, distance, entity, i);
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

					SortEntityByDistance(sortedEntities, distance, entity, i);
				}
				else
				{
					Math::vec3 cameraPosition = renderPacket.MainCameraWorldSpaceTranslation;
					float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedEntities, distance, entity, i);
				}

				i++;
			}
		}

		RenderTime& renderTime = Renderer::GetRenderTime();
		renderTime.PreGeometryPassSortTime += timer.ElapsedMS();

		return sortedEntities;
	}

	void SceneRenderer::SortEntityByDistance(std::map<float, Entity>& sortedEntities, float distance, Entity entity, uint32_t offset)
	{
		if (sortedEntities.find(distance) == sortedEntities.end())
		{
			sortedEntities[distance] = entity;
			return;
		}

		// slightly modify the distance
		sortedEntities[distance + (0.01f * offset)] = entity;
	}

	void SceneRenderer::GeometryPass(const SceneRenderPacket& renderPacket, const std::map<float, Entity>& sortedEntities)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		InstrumentationTimer timer("Geometry Pass");
		SceneLightDescription sceneLightDesc = Renderer::GetSceneLightDescription();
		
		// Render in reverse to blend correctly
		for (auto it = sortedEntities.crbegin(); it != sortedEntities.crend(); it++)
		{
			Entity entity = it->second;

			VX_CORE_ASSERT(entity.HasComponent<MeshRendererComponent>() || entity.HasComponent<StaticMeshRendererComponent>(), "Entity doesn't have mesh component!");

			if (entity.HasComponent<MeshRendererComponent>())
			{
				RenderMesh(scene, entity, sceneLightDesc);
			}
			else if (entity.HasComponent<StaticMeshRendererComponent>())
			{
				RenderStaticMesh(scene, entity, sceneLightDesc);
			}
		}

		RenderTime& renderTime = Renderer::GetRenderTime();
		renderTime.GeometryPassRenderTime += timer.ElapsedMS();
	}

	void SceneRenderer::RenderMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc)
	{
		VX_PROFILE_FUNCTION();

		const MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

		AssetHandle meshHandle = meshRendererComponent.Mesh;
		if (!AssetManager::IsHandleValid(meshHandle))
			return;

		SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
		if (!mesh)
			return;

		Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);
		auto& submesh = mesh->GetSubmesh();

		SharedReference<Material> material = submesh.GetMaterial();
		if (!material)
			return;

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

	void SceneRenderer::RenderStaticMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc)
	{
		VX_PROFILE_FUNCTION();

		const StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

		AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
		if (!AssetManager::IsHandleValid(staticMeshHandle))
			return;

		SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
		if (!staticMesh)
			return;

		Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);
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

	void SceneRenderer::SetEnvironment(AssetHandle environmentHandle, SkyboxComponent& skyboxComponent, SharedReference<Skybox>& environment)
	{
		VX_PROFILE_FUNCTION();

		s_EnvironmentHandle = environmentHandle;
		Renderer::CreateEnvironmentMap(skyboxComponent, environment);
		Renderer::SetEnvironment(environment);
	}

	void SceneRenderer::ClearEnvironment()
	{
		VX_PROFILE_FUNCTION();

		Renderer::SetEnvironment(s_EmptyEnvironment);
		s_EnvironmentHandle = 0;
	}

	void SceneRenderer::RenderEnvironment(const Math::mat4& view, const Math::mat4& projection, SkyboxComponent* skyboxComponent, SharedReference<Skybox>& environment)
	{
		VX_PROFILE_FUNCTION();

		Renderer::DrawEnvironmentMap(view, projection, *skyboxComponent, environment);
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
