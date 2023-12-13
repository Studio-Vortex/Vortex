#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Core/Thread.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Framebuffer.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

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

	void SceneRenderer::OnRenderScene2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		BeginScene2D(renderPacket);

		LightPass2D(renderPacket);

		SpritePass2D(renderPacket);

		ParticlePass2D(renderPacket);

		TextPass2D(renderPacket);

		// Scene Gizmos
		if (renderPacket.EditorScene)
		{
			SharedReference<Project> project = Project::GetActive();
			const ProjectProperties& properties = project->GetProperties();

			if (properties.RendererProps.DisplaySceneIconsInEditor)
			{
				SceneGizmosPass2D(renderPacket);
			}
		}

		EndScene2D();
	}

	void SceneRenderer::OnRenderScene3D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		std::map<float, Entity> sortedGeometry;

		Thread sortThread([&]() {
			SortMeshGeometry(renderPacket, sortedGeometry);
		});

		const Math::mat4* view = (const Math::mat4*)&renderPacket.MainCameraViewMatrix;
		const Math::mat4* projection = (const Math::mat4*)&renderPacket.MainCameraProjectionMatrix;

		SkyboxComponent skyboxComponent;
		SharedReference<Skybox> environment = nullptr;

		FindCurrentEnvironment(renderPacket, skyboxComponent, environment);

		BeginScene(renderPacket);

		const bool foundEnvironment = environment != nullptr;
		const bool hasSceneCamera = (view != nullptr && projection != nullptr);
		const bool hasEnvironment = hasSceneCamera && foundEnvironment;

		if (hasEnvironment)
		{
			RenderEnvironment(*view, *projection, &skyboxComponent, environment);
		}
		else
		{
			ClearEnvironment();
		}

		LightPass(renderPacket);

		if (sortThread.Joinable()) {
			sortThread.Join();
		}

		GeometryPass(renderPacket, sortedGeometry);

		EndScene();
	}

	void SceneRenderer::BeginScene2D(const SceneRenderPacket& renderPacket)
	{
		if (renderPacket.EditorScene)
		{
			EditorCamera* camera = (EditorCamera*)renderPacket.MainCamera;

			Renderer2D::BeginScene(camera);
		}
		else
		{
			const SceneCamera& camera = (const SceneCamera&)*renderPacket.MainCamera;
			const Math::mat4 view = renderPacket.MainCameraViewMatrix;

			Renderer2D::BeginScene(camera, view);
		}
	}

	void SceneRenderer::EndScene2D()
	{
		Renderer2D::EndScene();
	}

	void SceneRenderer::LightPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto view = scene->GetAllEntitiesWith<TransformComponent, LightSource2DComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, scene };
			const auto [transformComponent, lightSource2DComponent] = view.get<TransformComponent, LightSource2DComponent>(e);

			if (!entity.IsActive())
				continue;

			Renderer2D::RenderLightSource(transformComponent, lightSource2DComponent);
		}
	}

	void SceneRenderer::SpritePass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		// Sprite Pass 2D
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, scene };
				const auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(e);

				if (!entity.IsActive())
					continue;

				if (!spriteRendererComponent.Visible)
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
			auto view = scene->GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, scene };
				const auto [transformComponent, circleRendererComponent] = view.get<TransformComponent, CircleRendererComponent>(e);

				if (!entity.IsActive())
					continue;

				if (!circleRendererComponent.Visible)
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

	void SceneRenderer::ParticlePass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;
		Math::mat4 cameraView = renderPacket.MainCameraViewMatrix;

		auto view = scene->GetAllEntitiesWith<TransformComponent, ParticleEmitterComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, scene };
			if (!entity.IsActive())
				continue;

			const auto& pmc = entity.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
				continue;

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				continue;

			const auto& particles = particleEmitter->GetParticles();
			const bool random = particleEmitter->GetProperties().GenerateRandomColors;

			// Render the particles in reverse to blend correctly
			for (auto it = particles.crbegin(); it != particles.crend(); it++)
			{
				const Particle& particle = *it;

				if (!particle.Active)
					continue;

				const float life = particle.LifeRemaining / particle.LifeTime;
				Math::vec2 size = Math::Lerp(particle.SizeEnd, particle.SizeBegin, life);
				Math::vec4 color;

				if (random)
					color = particle.RandomColor;
				else
					color = Math::Lerp(particle.ColorEnd, particle.ColorBegin, life);

				Renderer2D::DrawQuadBillboard(cameraView, particle.Position, size, color);
			}
		}
	}

	void SceneRenderer::TextPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto view = scene->GetAllEntitiesWith<TransformComponent, TextMeshComponent>();

		RendererAPI::TriangleCullMode originalCullMode = Renderer2D::GetCullMode();
		Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::None);

		for (const auto e : view)
		{
			Entity entity{ e, scene };
			const auto [transformComponent, textMeshComponent] = view.get<TransformComponent, TextMeshComponent>(e);

			if (!entity.IsActive())
				continue;

			if (!textMeshComponent.Visible)
				continue;

			const AssetHandle fontHandle = textMeshComponent.FontAsset;
			SharedReference<Font> font = nullptr;

			if (AssetManager::IsHandleValid(fontHandle))
			{
				font = AssetManager::GetAsset<Font>(fontHandle);
			}
			else
			{
				font = Font::GetDefaultFont();
			}

			const Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);

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

	void SceneRenderer::SceneGizmosPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(renderPacket.EditorScene, "Scene Gizmos can only be rendered in a Editor Scene!");

		Scene* scene = renderPacket.Scene;
		const Math::mat4 cameraView = renderPacket.MainCameraViewMatrix;

		const ProjectProperties& projectProps = Project::GetActive()->GetProperties();

		// Camera Gizmos
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, CameraComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, scene };
				const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);

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

		// Light Gizmos
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, scene };
				const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(e);

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

		// Audio Gizmos
		{
			auto view = scene->GetAllEntitiesWith<TransformComponent, AudioSourceComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, scene };
				const auto [transformComponent, audioSourceComponent] = view.get<TransformComponent, AudioSourceComponent>(e);

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

	void SceneRenderer::BeginScene(const SceneRenderPacket& renderPacket)
	{
		SharedReference<Framebuffer> framebuffer = renderPacket.TargetFramebuffer;

		if (renderPacket.EditorScene)
		{
			EditorCamera* camera = (EditorCamera*)renderPacket.MainCamera;

			Renderer::BeginScene(camera, framebuffer);
		}
		else
		{
			const SceneCamera& camera = (const SceneCamera&)*renderPacket.MainCamera;
			const Math::mat4& view = renderPacket.MainCameraViewMatrix;
			const Math::vec3& translation = renderPacket.MainCameraWorldSpaceTranslation;

			Renderer::BeginScene(camera, view, translation, framebuffer);
		}
	}

	void SceneRenderer::EndScene()
	{
		Renderer::EndScene();
	}

	void SceneRenderer::LightPass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto lightSourceView = scene->GetAllEntitiesWith<TransformComponent, LightSourceComponent>();

		for (const auto e : lightSourceView)
		{
			Entity entity{ e, scene };
			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();

			if (!entity.IsActive())
				continue;

			if (!lightSourceComponent.Visible)
				continue;

			Renderer::RenderLightSource(scene->GetWorldSpaceTransform(entity), lightSourceComponent);
		}
	}

	void SceneRenderer::SortMeshGeometry(const SceneRenderPacket& renderPacket, std::map<float, Entity>& sortedGeometry)
	{
		VX_PROFILE_FUNCTION();

		InstrumentationTimer timer("Pre-Geo-Pass Sort");
		Scene* scene = renderPacket.Scene;

		// Sort All Meshes by distance from camera

		// Sort Meshes
		{
			auto meshRendererView = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
			uint32_t i = 0;

			for (const auto e : meshRendererView)
			{
				Entity entity{ e, scene };
				const MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

				if (!entity.IsActive())
					continue;

				if (!meshRendererComponent.Visible)
					continue;

				const Math::vec3 entityWorldSpaceTranslation = scene->GetWorldSpaceTransform(entity).Translation;

				if (renderPacket.EditorScene)
				{
					const EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
					const Math::vec3 cameraPosition = editorCamera->GetPosition();
					const float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedGeometry, distance, entity, i);
				}
				else
				{
					const Math::vec3 cameraPosition = renderPacket.MainCameraWorldSpaceTranslation;
					const float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedGeometry, distance, entity, i);
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
				const StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

				if (!entity.IsActive())
					continue;

				if (!staticMeshRendererComponent.Visible)
					continue;

				const Math::vec3 entityWorldSpaceTranslation = scene->GetWorldSpaceTransform(entity).Translation;

				if (renderPacket.EditorScene)
				{
					const EditorCamera* editorCamera = (EditorCamera*)renderPacket.MainCamera;
					const Math::vec3 cameraPosition = editorCamera->GetPosition();
					const float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedGeometry, distance, entity, i);
				}
				else
				{
					const Math::vec3 cameraPosition = renderPacket.MainCameraWorldSpaceTranslation;
					const float distance = Math::Distance(cameraPosition, entityWorldSpaceTranslation);

					SortEntityByDistance(sortedGeometry, distance, entity, i);
				}

				i++;
			}
		}

		RenderTime& renderTime = Renderer::GetRenderTime();
		renderTime.PreGeometryPassSortTime += timer.ElapsedMS();
	}

	void SceneRenderer::SortEntityByDistance(std::map<float, Entity>& sortedEntities, float distance, Entity entity, uint32_t offset)
	{
		std::scoped_lock<std::mutex> lock(m_GeometrySortMutex);
		if (sortedEntities.find(distance) == sortedEntities.end())
		{
			sortedEntities[distance] = entity;
			return;
		}

		// slightly modify the distance
		sortedEntities[distance + (0.01f * offset)] = entity;
	}

	void SceneRenderer::GeometryPass(const SceneRenderPacket& renderPacket, const std::map<float, Entity>& sortedGeometry)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		InstrumentationTimer timer("Geometry Pass");
		SceneLightDescription sceneLightDesc = Renderer::GetSceneLightDescription();
		
		// Render in reverse to blend correctly
		for (auto it = sortedGeometry.crbegin(); it != sortedGeometry.crend(); it++)
		{
			Entity entity = it->second;

			if (!entity)
				continue;
			
			const bool hasRequiredComponent = entity.HasAny<MeshRendererComponent, StaticMeshRendererComponent>();
			VX_CORE_ASSERT(hasRequiredComponent, "Entity doesn't have mesh component!");

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
		if (mesh == nullptr)
			return;

		Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);
		auto& submesh = mesh->GetSubmesh();

		SharedReference<Material> material = submesh.GetMaterial();
		if (material == nullptr)
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

		const bool isAnimated = mesh->HasAnimations();
		const bool hasRequiredComponents = entity.HasComponent<AnimatorComponent, AnimationComponent>();

		shader->SetBool("u_HasAnimations", isAnimated);

		if (isAnimated && hasRequiredComponents)
		{
			const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
			const std::vector<Math::mat4>& transforms = animatorComponent.Animator->GetFinalBoneMatrices();
			const uint32_t size = transforms.size();

			for (uint32_t i = 0; i < size; i++)
			{
				shader->SetMat4("u_FinalBoneMatrices[" + std::to_string(i) + "]", transforms[i]);
			}
		}

		submesh.Render();

		ResetMaterialFlags();
	}

	void SceneRenderer::RenderStaticMesh(Scene* scene, Entity entity, const SceneLightDescription& sceneLightDesc)
	{
		VX_PROFILE_FUNCTION();

		const StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

		AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
		if (!AssetManager::IsHandleValid(staticMeshHandle))
			return;

		SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
		if (staticMesh == nullptr)
			return;

		Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(entity);
		const auto& submeshes = staticMesh->GetSubmeshes();

		SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

		// render each submesh
		for (const auto& [submeshIndex, submesh] : submeshes)
		{
			VX_CORE_ASSERT(materialTable->HasMaterial(submeshIndex), "Material table not synchronized with mesh!");

			AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
			if (!AssetManager::IsHandleValid(materialHandle))
				continue;

			SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
			if (material == nullptr)
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

			submesh.Render(materialHandle);

			ResetMaterialFlags();
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

			const bool consistent = environmentHandle == s_EnvironmentHandle;
			if (consistent)
				continue;
			
			SetEnvironment(environmentHandle, skyboxComponent, environment);
			break;
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

		s_EnvironmentHandle = 0;
		Renderer::SetEnvironment(s_EmptyEnvironment);
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

	void SceneRenderer::ResetMaterialFlags()
	{
		RendererAPI::TriangleCullMode cullMode = Renderer::GetCullMode();
		RenderCommand::SetCullMode(cullMode);
		RenderCommand::EnableDepthTest();
	}

}
