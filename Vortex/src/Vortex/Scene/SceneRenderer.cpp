#include "vxpch.h"
#include "SceneRenderer.h"

#include "Vortex/Core/Thread.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"
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

		if (renderPacket.IsEditorScene)
		{
			SharedReference<Project> project = Project::GetActive();
			const ProjectProperties& properties = project->GetProperties();

			if (properties.RendererProps.DisplaySceneIconsInEditor)
			{
				SceneGizmosPass2D(renderPacket);
			}
		}
		else
		{
			DebugRenderPass2D(renderPacket);
		}

		EndScene2D();
	}

	void SceneRenderer::OnRenderScene3D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		std::map<float, Actor> sortedGeometry;

		Thread sortThread([&]() {
			SortMeshGeometry(renderPacket, sortedGeometry);
		});

		const Math::mat4* view = (const Math::mat4*)&renderPacket.PrimaryCameraViewMatrix;
		const Math::mat4* projection = (const Math::mat4*)&renderPacket.PrimaryCameraProjectionMatrix;

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

		EmissiveMeshPass(renderPacket);

		if (sortThread.Joinable()) {
			sortThread.Join();
		}

		GeometryPass(renderPacket, sortedGeometry);

		EndScene();
	}

	void SceneRenderer::BeginScene2D(const SceneRenderPacket& renderPacket)
	{
		if (renderPacket.IsEditorScene)
		{
			EditorCamera* camera = (EditorCamera*)renderPacket.PrimaryCamera;

			Renderer2D::BeginScene(camera);
		}
		else
		{
			const SceneCamera& camera = (const SceneCamera&)*renderPacket.PrimaryCamera;
			const Math::mat4& view = renderPacket.PrimaryCameraViewMatrix;

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

		auto view = scene->GetAllActorsWith<TransformComponent, LightSource2DComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, scene };
			const auto [transformComponent, lightSource2DComponent] = view.get<TransformComponent, LightSource2DComponent>(e);

			if (!actor.IsActive())
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
			auto view = scene->GetAllActorsWith<TransformComponent, SpriteRendererComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, scene };
				const auto [transformComponent, spriteRendererComponent] = view.get<TransformComponent, SpriteRendererComponent>(e);

				if (!actor.IsActive())
					continue;

				if (!spriteRendererComponent.Visible)
					continue;

				AssetHandle textureHandle = spriteRendererComponent.Texture;
				SharedReference<Texture2D> texture = nullptr;

				if (AssetManager::IsHandleValid(textureHandle))
					texture = AssetManager::GetAsset<Texture2D>(textureHandle);

				Renderer2D::DrawSprite(
					scene->GetWorldSpaceTransformMatrix(actor),
					spriteRendererComponent,
					texture,
					(int)(entt::entity)e
				);
			}
		}

		// Circle Pass 2D
		{
			auto view = scene->GetAllActorsWith<TransformComponent, CircleRendererComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, scene };
				const auto [transformComponent, circleRendererComponent] = view.get<TransformComponent, CircleRendererComponent>(e);

				if (!actor.IsActive())
					continue;

				if (!circleRendererComponent.Visible)
					continue;

				Renderer2D::DrawCircle(
					scene->GetWorldSpaceTransformMatrix(actor),
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
		const Math::mat4& cameraView = renderPacket.PrimaryCameraViewMatrix;

		auto view = scene->GetAllActorsWith<TransformComponent, ParticleEmitterComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, scene };
			if (!actor.IsActive())
				continue;

			const ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();
			if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
				continue;

			SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(pmc.EmitterHandle);
			if (!particleEmitter)
				continue;

			const std::vector<Particle>& particles = particleEmitter->GetParticles();
			const bool random = particleEmitter->GetProperties().GenerateRandomColors;

			// Render the particles in reverse to blend correctly
			for (auto it = particles.crbegin(); it != particles.crend(); it++)
			{
				const Particle& particle = *it;

				if (!particle.Active)
					continue;

				const float particleLife = particle.LifeRemaining / particle.LifeTime;
				const Math::vec2 size = Math::Lerp(particle.SizeEnd, particle.SizeBegin, particleLife);
				Math::vec4 color;

				if (random)
				{
					color = particle.RandomColor;
				}
				else
				{
					color = Math::Lerp(particle.ColorEnd, particle.ColorBegin, particleLife);
				}

				Renderer2D::DrawQuadBillboard(
					cameraView,
					particle.Position,
					size,
					color,
					(int)(entt::entity)e
				);
			}
		}
	}

	void SceneRenderer::TextPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto view = scene->GetAllActorsWith<TransformComponent, TextMeshComponent>();

		RendererAPI::TriangleCullMode originalCullMode = Renderer2D::GetCullMode();
		Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::None);

		for (const auto e : view)
		{
			Actor actor{ e, scene };
			const auto [transformComponent, textMeshComponent] = view.get<TransformComponent, TextMeshComponent>(e);

			if (!actor.IsActive())
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

			const Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(actor);

			Renderer2D::DrawString(
				textMeshComponent.TextString,
				font,
				worldSpaceTransform,
				textMeshComponent.MaxWidth,
				textMeshComponent.Color,
				textMeshComponent.BackgroundColor,
				textMeshComponent.LineSpacing,
				textMeshComponent.Kerning,
				(int)(entt::entity)e
			);

			if (!textMeshComponent.DropShadow.Enabled)
				continue;

			const Math::mat4 shadowOffset = Math::Translate({ textMeshComponent.DropShadow.ShadowDistance, 0.01f })
				* Math::Scale(Math::vec3(textMeshComponent.DropShadow.ShadowScale));

			Renderer2D::DrawString(
				textMeshComponent.TextString,
				font,
				worldSpaceTransform * shadowOffset,
				textMeshComponent.MaxWidth,
				textMeshComponent.DropShadow.Color,
				textMeshComponent.BackgroundColor,
				textMeshComponent.LineSpacing,
				textMeshComponent.Kerning,
				(int)(entt::entity)e
			);
		}

		Renderer2D::SetCullMode(originalCullMode);
	}

	void SceneRenderer::DebugRenderPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(!renderPacket.IsEditorScene, "DebugRenderPass2D can only be called in a non Editor Scene!");

		Scene* scene = renderPacket.Scene;

		// Invoke Actor.OnDebugRender
		auto view = scene->GetAllActorsWith<ScriptComponent>();
		for (const auto e : view)
		{
			Actor actor{ e, scene };

			if (!actor.IsActive())
				continue;

			actor.CallMethod(ScriptMethod::OnDebugRender);
		}
	}

	void SceneRenderer::SceneGizmosPass2D(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(renderPacket.IsEditorScene, "Scene Gizmos can only be rendered in a Editor Scene!");

		Scene* scene = renderPacket.Scene;
		const Math::mat4& cameraView = renderPacket.PrimaryCameraViewMatrix;

		const ProjectProperties& projectProps = Project::GetActive()->GetProperties();
		const Math::vec2 gizmoSize = Math::vec2(projectProps.GizmoProps.GizmoSize);
		const Math::vec4 gizmoColor = ColorToVec4(Color::White);

		// Camera Gizmos
		{
			auto view = scene->GetAllActorsWith<TransformComponent, CameraComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, scene };
				const auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(e);

				if (!actor.IsActive())
					continue;

				const TransformComponent& transform = scene->GetWorldSpaceTransform(actor);

				Renderer2D::DrawQuadBillboard(
					cameraView,
					transform.Translation,
					EditorResources::CameraIcon,
					gizmoSize,
					gizmoColor,
					(int)(entt::entity)e
				);
			}
		}

		// Light Gizmos
		{
			auto view = scene->GetAllActorsWith<TransformComponent, LightSourceComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, scene };
				const auto [transformComponent, lightSourceComponent] = view.get<TransformComponent, LightSourceComponent>(e);

				if (!actor.IsActive())
					continue;

				const TransformComponent& transform = scene->GetWorldSpaceTransform(actor);

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
					gizmoSize,
					gizmoColor,
					(int)(entt::entity)e
				);
			}
		}

		// Audio Gizmos
		{
			auto view = scene->GetAllActorsWith<TransformComponent, AudioSourceComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, scene };
				const auto [transformComponent, audioSourceComponent] = view.get<TransformComponent, AudioSourceComponent>(e);

				if (!actor.IsActive())
					continue;

				const TransformComponent& transform = scene->GetWorldSpaceTransform(actor);

				Renderer2D::DrawQuadBillboard(
					cameraView,
					transform.Translation,
					EditorResources::AudioSourceIcon,
					gizmoSize,
					gizmoColor,
					(int)(entt::entity)e
				);
			}
		}
	}

	void SceneRenderer::BeginScene(const SceneRenderPacket& renderPacket)
	{
		SharedReference<Framebuffer> framebuffer = renderPacket.TargetFramebuffer;

		PreparePostProcess(renderPacket);

		if (renderPacket.IsEditorScene)
		{
			EditorCamera* camera = (EditorCamera*)renderPacket.PrimaryCamera;

			Renderer::BeginScene(camera, framebuffer);
		}
		else
		{
			const SceneCamera& camera = (const SceneCamera&)*renderPacket.PrimaryCamera;
			const Math::mat4& view = renderPacket.PrimaryCameraViewMatrix;
			const Math::vec3& translation = renderPacket.PrimaryCameraWorldSpaceTranslation;

			Renderer::BeginScene(camera, view, translation, framebuffer);
		}
	}

	void SceneRenderer::EndScene()
	{
		Renderer::EndScene();
	}

	void SceneRenderer::PreparePostProcess(const SceneRenderPacket& renderPacket)
	{
		Scene* scene = renderPacket.Scene;
		Actor primaryCamera = scene->GetPrimaryCameraActor();

		if (!primaryCamera)
		{
			return;
		}

		const CameraComponent& cameraComponent = primaryCamera.GetComponent<CameraComponent>();
		const CameraComponent::PostProcessInfo& postProcessInfo = cameraComponent.PostProcessing;

		if (postProcessInfo.Enabled)
		{
			const CameraComponent::PostProcessInfo::BloomInfo& bloomInfo = postProcessInfo.Bloom;

			if (bloomInfo.Enabled)
			{
				if (!Renderer::IsFlagSet(RenderFlag::EnableBloom))
				{
					Renderer::SetFlag(RenderFlag::EnableBloom);
				}

				Renderer::SetBloomThreshold(bloomInfo.Threshold);
				Renderer::SetBloomKnee(bloomInfo.Knee);
				Renderer::SetBloomIntensity(bloomInfo.Intensity);
			}
			else
			{
				if (Renderer::IsFlagSet(RenderFlag::EnableBloom))
				{
					Renderer::DisableFlag(RenderFlag::EnableBloom);
				}
			}
		}
	}

	void SceneRenderer::LightPass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto lightSourceView = scene->GetAllActorsWith<TransformComponent, LightSourceComponent>();

		for (const auto e : lightSourceView)
		{
			Actor actor{ e, scene };
			const LightSourceComponent& lsc = actor.GetComponent<LightSourceComponent>();

			if (!actor.IsActive())
				continue;

			if (!lsc.Visible)
				continue;

			TransformComponent transform = scene->GetWorldSpaceTransform(actor);

			Renderer::RenderLightSource(transform, lsc);
		}
	}

	void SceneRenderer::EmissiveMeshPass(const SceneRenderPacket& renderPacket)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		auto meshView = scene->GetAllActorsWith<TransformComponent, MeshRendererComponent>();

		for (const auto e : meshView)
		{
			Actor actor{ e, scene };
			const MeshRendererComponent& mrc = actor.GetComponent<MeshRendererComponent>();

			if (!actor.IsActive())
				continue;

			if (!AssetManager::IsHandleValid(mrc.Mesh))
				continue;

			SharedReference<MaterialTable> materialTable = mrc.Materials;
			VX_CORE_ASSERT(materialTable, "invalid material table!");
			const uint32_t materialCount = materialTable->GetMaterialCount();

			const Math::vec3 translation = scene->GetWorldSpaceTransform(actor).Translation;

			for (uint32_t i = 0; i < materialCount; i++)
			{
				AssetHandle materialHandle = materialTable->GetMaterial(i);
				if (!AssetManager::IsHandleValid(materialHandle))
					continue;

				SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (material == nullptr)
					continue;

				const float emission = material->GetEmission();

				// emission should never be less than zero but this is just in case
				if (emission <= 0.0f)
					continue;

				const Math::vec3& radiance = material->GetAlbedo();
				const float intensity = emission;

				Renderer::RenderEmissiveMaterial(translation, radiance, intensity);
			}
		}

		auto staticMeshView = scene->GetAllActorsWith<TransformComponent, StaticMeshRendererComponent>();

		for (const auto e : staticMeshView)
		{
			Actor actor{ e, scene };
			const StaticMeshRendererComponent& smrc = actor.GetComponent<StaticMeshRendererComponent>();

			if (!actor.IsActive())
				continue;

			if (!AssetManager::IsHandleValid(smrc.StaticMesh))
				continue;

			SharedReference<MaterialTable> materialTable = smrc.Materials;
			VX_CORE_ASSERT(materialTable, "invalid material table!");
			const uint32_t materialCount = materialTable->GetMaterialCount();

			const Math::vec3 translation = scene->GetWorldSpaceTransform(actor).Translation;

			for (uint32_t i = 0; i < materialCount; i++)
			{
				AssetHandle materialHandle = materialTable->GetMaterial(i);
				if (!AssetManager::IsHandleValid(materialHandle))
					continue;

				SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
				if (material == nullptr)
					continue;

				const float emission = material->GetEmission();

				// emission should never be less than zero but this is just in case
				if (emission <= 0.0f)
					continue;

				const Math::vec3& radiance = material->GetAlbedo();
				const float intensity = emission;

				Renderer::RenderEmissiveMaterial(translation, radiance, intensity);
			}
		}
	}

	void SceneRenderer::SortMeshGeometry(const SceneRenderPacket& renderPacket, std::map<float, Actor>& sortedGeometry)
	{
		VX_PROFILE_FUNCTION();

		InstrumentationTimer timer("Pre-Geo-Pass Sort");
		Scene* scene = renderPacket.Scene;

		// Sort All Meshes by distance from camera
		{
			auto meshRendererView = scene->GetAllActorsWith<TransformComponent, MeshRendererComponent>();
			uint32_t i = 0;

			for (const auto e : meshRendererView)
			{
				Actor actor{ e, scene };
				const MeshRendererComponent& mrc = actor.GetComponent<MeshRendererComponent>();

				if (!actor.IsActive())
					continue;

				if (!mrc.Visible)
					continue;

				const Math::vec3 worldSpaceTranslation = scene->GetWorldSpaceTransform(actor).Translation;

				if (renderPacket.IsEditorScene)
				{
					const EditorCamera* editorCamera = (EditorCamera*)renderPacket.PrimaryCamera;
					const Math::vec3& cameraPosition = editorCamera->GetPosition();
					const float distance = Math::Distance(cameraPosition, worldSpaceTranslation);

					SortActorByDistance(sortedGeometry, distance, actor, i);
				}
				else
				{
					const Math::vec3& cameraPosition = renderPacket.PrimaryCameraWorldSpaceTranslation;
					const float distance = Math::Distance(cameraPosition, worldSpaceTranslation);

					SortActorByDistance(sortedGeometry, distance, actor, i);
				}

				i++;
			}
		}

		// Sort Static Meshes
		{
			auto staticMeshRendererView = scene->GetAllActorsWith<TransformComponent, StaticMeshRendererComponent>();
			uint32_t i = 0;

			for (const auto e : staticMeshRendererView)
			{
				Actor actor{ e, scene };
				const StaticMeshRendererComponent& smrc = actor.GetComponent<StaticMeshRendererComponent>();

				if (!actor.IsActive())
					continue;

				if (!smrc.Visible)
					continue;

				const Math::vec3 worldSpaceTranslation = scene->GetWorldSpaceTransform(actor).Translation;

				if (renderPacket.IsEditorScene)
				{
					const EditorCamera* editorCamera = (EditorCamera*)renderPacket.PrimaryCamera;
					const Math::vec3& cameraPosition = editorCamera->GetPosition();
					const float distance = Math::Distance(cameraPosition, worldSpaceTranslation);

					SortActorByDistance(sortedGeometry, distance, actor, i);
				}
				else
				{
					const Math::vec3& cameraPosition = renderPacket.PrimaryCameraWorldSpaceTranslation;
					const float distance = Math::Distance(cameraPosition, worldSpaceTranslation);

					SortActorByDistance(sortedGeometry, distance, actor, i);
				}

				i++;
			}
		}

		RenderTime& renderTime = Renderer::GetRenderTime();
		renderTime.PreGeometryPassSortTime += timer.ElapsedMS();
	}

	void SceneRenderer::SortActorByDistance(std::map<float, Actor>& sortedActors, float distance, Actor actor, uint32_t offset)
	{
		std::scoped_lock<std::mutex> lock(m_GeometrySortMutex);
		if (sortedActors.find(distance) == sortedActors.end())
		{
			sortedActors[distance] = actor;
			return;
		}

		// slightly modify the distance
		sortedActors[distance + (0.01f * offset)] = actor;
	}

	void SceneRenderer::GeometryPass(const SceneRenderPacket& renderPacket, const std::map<float, Actor>& sortedGeometry)
	{
		VX_PROFILE_FUNCTION();

		Scene* scene = renderPacket.Scene;

		InstrumentationTimer timer("Geometry Pass");
		SceneLightDescription sceneLightDesc = Renderer::GetSceneLightDescription();
		
		// Render in reverse to blend correctly
		for (auto it = sortedGeometry.crbegin(); it != sortedGeometry.crend(); it++)
		{
			Actor actor = it->second;

			if (!actor)
				continue;
			
			const bool hasRequiredComponent = actor.HasAny<MeshRendererComponent, StaticMeshRendererComponent>();
			VX_CORE_ASSERT(hasRequiredComponent, "Actor doesn't have mesh component!");

			if (actor.HasComponent<MeshRendererComponent>())
			{
				RenderMesh(scene, actor, sceneLightDesc);
			}
			else if (actor.HasComponent<StaticMeshRendererComponent>())
			{
				RenderStaticMesh(scene, actor, sceneLightDesc);
			}
		}

		RenderTime& renderTime = Renderer::GetRenderTime();
		renderTime.GeometryPassRenderTime += timer.ElapsedMS();
	}

	void SceneRenderer::RenderMesh(Scene* scene, Actor actor, const SceneLightDescription& sceneLightDesc)
	{
		VX_PROFILE_FUNCTION();

		const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();

		AssetHandle meshHandle = meshRendererComponent.Mesh;
		if (!AssetManager::IsHandleValid(meshHandle))
			return;

		SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
		if (mesh == nullptr)
			return;

		const Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(actor);
		const auto& submesh = mesh->GetSubmesh();

		SharedReference<Material> material = submesh.GetMaterial();
		if (material == nullptr)
			return;

		SetMaterialFlags(material);

		SharedReference<Shader> shader = material->GetShader();
		shader->Enable();

		shader->SetBool("u_SceneProperties.HasSkyLight", sceneLightDesc.HasSkyLight);
		shader->SetInt("u_SceneProperties.ActivePointLights", sceneLightDesc.ActivePointLights);
		shader->SetInt("u_SceneProperties.ActiveSpotLights", sceneLightDesc.ActiveSpotLights);
		shader->SetInt("u_SceneProperties.ActiveEmissiveMeshes", sceneLightDesc.ActiveEmissiveMeshes);
		shader->SetMat4("u_Model", worldSpaceTransform); // should be submesh world transform

		Renderer::BindSkyLightDepthMap();
		Renderer::BindPointLightDepthMaps();
		Renderer::BindSpotLightDepthMaps();

		const bool isAnimated = mesh->HasAnimations();
		const bool hasRequiredComponents = actor.HasComponent<AnimatorComponent, AnimationComponent>();

		shader->SetBool("u_HasAnimations", isAnimated);

		if (isAnimated && hasRequiredComponents)
		{
			const AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
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

	void SceneRenderer::RenderStaticMesh(Scene* scene, Actor actor, const SceneLightDescription& sceneLightDesc)
	{
		VX_PROFILE_FUNCTION();

		const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();

		AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
		if (!AssetManager::IsHandleValid(staticMeshHandle))
			return;

		SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
		if (staticMesh == nullptr)
			return;

		const Math::mat4 worldSpaceTransform = scene->GetWorldSpaceTransformMatrix(actor);
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
			shader->SetInt("u_SceneProperties.ActiveEmissiveMeshes", sceneLightDesc.ActiveEmissiveMeshes);
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

		auto skyboxView = scene->GetAllActorsWith<SkyboxComponent>();

		// Only render one environment per scene
		for (const auto e : skyboxView)
		{
			Actor actor{ e, scene };

			if (!actor.IsActive())
				continue;

			skyboxComponent = actor.GetComponent<SkyboxComponent>();
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
		m_LastCullMode = Renderer::GetCullMode();

		if (material->HasFlag(MaterialFlag::NoDepthTest))
		{
			RenderCommand::DisableDepthTest();
			RenderCommand::SetCullMode(RendererAPI::TriangleCullMode::None);
		}
	}

	void SceneRenderer::ResetMaterialFlags()
	{
		RenderCommand::SetCullMode(m_LastCullMode);
		RenderCommand::EnableDepthTest();
	}

}
