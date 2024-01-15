#include "vxpch.h"
#include "Scene.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/String.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/SceneRenderer.h"
#include "Vortex/Scene/ScriptableActor.h"
#include "Vortex/Scene/ComponentUtils.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

#include "Vortex/UI/UISystem.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleSystem.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/System/SystemManager.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/2D/Physics2D.h"

#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Editor/SelectionManager.h"
#include "Vortex/Editor/UI/UI.h"

namespace Vortex {

	static SceneRenderer s_SceneRenderer;
	static Timer s_NullTimer = Timer("", 0.0f, nullptr);

	Scene::Scene(SharedReference<Framebuffer>& targetFramebuffer)
		: m_TargetFramebuffer(targetFramebuffer)
	{
		SystemManager::OnContextSceneCreated(this);

		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraConstruct>(this);
		m_Registry.on_construct<StaticMeshRendererComponent>().connect<&Scene::OnStaticMeshConstruct>(this);

		m_Registry.on_construct<ParticleEmitterComponent>().connect<&Scene::OnParticleEmitterConstruct>(this);
		m_Registry.on_destroy<ParticleEmitterComponent>().connect<&Scene::OnParticleEmitterDestruct>(this);

		m_Registry.on_construct<TextMeshComponent>().connect<&Scene::OnTextMeshConstruct>(this);
		
		m_Registry.on_construct<AnimatorComponent>().connect<&Scene::OnAnimatorConstruct>(this);
		m_Registry.on_construct<AnimationComponent>().connect<&Scene::OnAnimationConstruct>(this);

		m_Registry.on_construct<AudioSourceComponent>().connect<&Scene::OnAudioSourceConstruct>(this);
		m_Registry.on_destroy<AudioSourceComponent>().connect<&Scene::OnAudioSourceDestruct>(this);
		
		m_Registry.on_construct<AudioListenerComponent>().connect<&Scene::OnAudioListenerConstruct>(this);
		m_Registry.on_destroy<AudioListenerComponent>().connect<&Scene::OnAudioListenerDestruct>(this);
	}

	Scene::~Scene()
	{
		SystemManager::OnContextSceneDestroyed(this);

		m_Registry.on_construct<CameraComponent>().disconnect();
		m_Registry.on_construct<StaticMeshRendererComponent>().disconnect();

		m_Registry.on_construct<ParticleEmitterComponent>().disconnect();
		m_Registry.on_destroy<ParticleEmitterComponent>().disconnect();
		
		m_Registry.on_construct<TextMeshComponent>().disconnect();
		
		m_Registry.on_construct<AnimatorComponent>().disconnect();
		m_Registry.on_construct<AnimationComponent>().disconnect();
		
		m_Registry.on_construct<AudioSourceComponent>().disconnect();
		m_Registry.on_destroy<AudioSourceComponent>().disconnect();
		
		m_Registry.on_construct<AudioListenerComponent>().disconnect();
		m_Registry.on_destroy<AudioListenerComponent>().disconnect();
	}

	Actor Scene::CreateActor(const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		return CreateActorWithUUID(UUID(), name, marker);
	}

	Actor Scene::CreateChildActor(Actor parent, const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		Actor child = CreateActor(name, marker);

		ParentActor(child, parent);

		SortActors();

		return child;
	}

	Actor Scene::CreateActorWithUUID(UUID uuid, const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { m_Registry.create(), this };
		actor.AddComponent<IDComponent>(uuid);
		actor.AddComponent<TransformComponent>();

		TagComponent& tagComponent = actor.AddComponent<TagComponent>();
		tagComponent.Tag = name.empty() ? "Actor" : name;
		tagComponent.Marker = marker.empty() ? "Untagged" : marker;

		const HierarchyComponent& hierarchyComponent = actor.AddComponent<HierarchyComponent>();

		// Store the actor's UUID and the entt handle in our Actor map
		// actor here will be implicitly converted to an entt handle
		m_ActorMap[uuid] = actor;

		return actor;
	}

	Actor Scene::DuplicateActor(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		auto ParentActorFn = [&](Actor newActor) {
			if (actor.HasParent()) {
				ParentActor(newActor, actor.GetParent());
			}
		};

		if (actor.HasComponent<PrefabComponent>())
		{
			AssetHandle prefabHandle = actor.GetComponent<PrefabComponent>().Prefab;
			VX_CORE_ASSERT(AssetManager::IsHandleValid(prefabHandle), "invalid asset handle!");
			const TransformComponent& transform = actor.GetTransform();
			const Math::vec3& eulerRotation = transform.GetRotationEuler();
			Actor prefabInstance = Instantiate(AssetManager::GetAsset<Prefab>(prefabHandle), &transform.Translation, &eulerRotation, &transform.Scale);
			ParentActorFn(prefabInstance);
			return prefabInstance;
		}

		VX_CORE_ASSERT(actor.HasComponent<TagComponent>(), "all actors must have a tag component!");

		Actor duplicate = CreateActor(actor.Name(), actor.Marker());
		ParentActorFn(duplicate);

		// Copy components (except IDComponent and TagComponent)
		ComponentUtils::CopyComponentIfExists(AllComponents{}, duplicate, actor);

		duplicate.Children().clear();

		// Copy children actors
		// We must use an index based loop because the vector is modified below
		const std::vector<UUID>& children = actor.Children();
		const size_t numChildren = children.size();

		for (size_t i = 0; i < numChildren; i++)
		{
			Actor child = TryGetActorWithUUID(children[i]);
			if (!child)
				continue;

			Actor childDuplicate = DuplicateActor(child);

			// at this point childDuplicate is a child of actor, we need to remove it from actor
			UnparentActor(childDuplicate);
			ParentActor(childDuplicate, duplicate);
		}

		if (duplicate.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(actor, duplicate);
		}

		return duplicate;
	}

	Actor Scene::CreatePrefabActor(Actor prefabActor, Actor parent, const Math::vec3* translation, const Math::vec3* eulerRotation, const Math::vec3* scale)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_VERIFY(prefabActor.HasComponent<PrefabComponent>());

		Actor prefabInstance = CreateActor();
		if (parent) {
			prefabInstance.SetParentUUID(parent.GetUUID());
			parent.AddChild(prefabInstance.GetUUID());
		}

		prefabActor.m_Scene->CopyComponentIfExists<TagComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<TransformComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<CameraComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<SkyboxComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<LightSourceComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<MeshRendererComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<StaticMeshRendererComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<SpriteRendererComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<CircleRendererComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<ParticleEmitterComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<TextMeshComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<ButtonComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<AudioSourceComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<AudioListenerComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<RigidBodyComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<CharacterControllerComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<FixedJointComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<BoxColliderComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<SphereColliderComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<CapsuleColliderComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<MeshColliderComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<RigidBody2DComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<BoxCollider2DComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<CircleCollider2DComponent>(prefabInstance, m_Registry, prefabActor);
		prefabActor.m_Scene->CopyComponentIfExists<ScriptComponent>(prefabInstance, m_Registry, prefabActor);

		if (translation)
			prefabInstance.GetTransform().Translation = *translation;
		if (eulerRotation)
			prefabInstance.GetTransform().SetRotationEuler(*eulerRotation);
		if (scale)
			prefabInstance.GetTransform().Scale = *scale;

		for (UUID child : prefabActor.Children())
		{
			CreatePrefabActor(prefabActor.m_Scene->TryGetActorWithUUID(child), prefabInstance);
		}

		if (m_IsRunning)
		{
			if (prefabInstance.HasComponent<RigidBodyComponent>() && !Physics::IsPhysicsActor(prefabInstance.GetUUID()))
			{
				Physics::CreatePhysicsActor(prefabInstance);
			}
		}

		if (prefabInstance.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(prefabActor, prefabInstance);
		}

		return prefabInstance;
	}

	Actor Scene::Instantiate(SharedReference<Prefab> prefab, const Math::vec3* translation, const Math::vec3* eulerRotation, const Math::vec3* scale)
	{
		return InstantiateChild(prefab, {}, translation, eulerRotation, scale);
	}

	Actor Scene::InstantiateChild(SharedReference<Prefab> prefab, Actor parent, const Math::vec3* translation, const Math::vec3* eulerRotation, const Math::vec3* scale)
	{
		Actor result;

		// TODO we need a better way of getting the root actor
		prefab->m_Scene->m_Registry.each([&](auto actorID) {
			Actor actor{ actorID, prefab->m_Scene.Raw() };
			if (!actor.HasParent()) {
				result = CreatePrefabActor(actor, parent, translation, eulerRotation, scale);
			}
		});

		PrefabComponent& prefabComponent = result.AddComponent<PrefabComponent>();
		prefabComponent.Prefab = prefab->Handle;

		return result;
	}

	void Scene::SubmitToDestroyActor(Actor actor, bool excludeChildren)
	{
		SubmitToPostUpdateQueue([=]() {
			DestroyActorInternal(actor, excludeChildren);
		});
	}

	void Scene::ClearActors()
	{
		m_Registry.clear();
	}

	void Scene::OnRuntimeStart(bool muteAudio)
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = true;

		ResizePrimaryCamera();

		OnPhysicsSimulationStart();

		// TODO: when we have AudioSources that play on
		// start we need to stop them here if muteAudio is true
		SystemManager::OnRuntimeStart(this);

		// C# Actor Lifecycle
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = GetAllActorsWith<ScriptComponent>();

			// TODO we need to create the script instance if the actor wasn't active during OnRuntimeStart

			// Create all script instances
			for (const auto e : view)
			{
				Actor actor{ e, this };

				if (!actor.IsActive())
					continue;

				if (!ScriptEngine::IsScriptClassValid(actor))
					continue;

				if (!ScriptEngine::IsScriptComponentEnabled(actor))
					continue;

				ScriptEngine::RT_CreateActorScriptInstance(actor);
			}

			// Invoke Actor.OnAwake
			for (const auto e : view)
			{
				Actor actor{ e, this };

				if (!actor.IsActive())
					continue;

				actor.CallMethod(ScriptMethod::OnAwake);
			}

			// Invoke Actor.OnReset - Editor Only
			if (!Application::Get().IsRuntime())
			{
				for (const auto e : view)
				{
					Actor actor{ e, this };

					if (!actor.IsActive())
						continue;

					actor.CallMethod(ScriptMethod::OnReset);
				}
			}

			// Invoke Actor.OnCreate
			for (const auto e : view)
			{
				Actor actor{ e, this };

				if (!actor.IsActive())
					continue;

				actor.CallMethod(ScriptMethod::OnCreate);
			}
		}

		// C++ Actor Lifecycle
		{
			GetAllActorsWith<NativeScriptComponent>().each([=](auto actorID, auto& nsc)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Actor = Actor{ actorID, this };
				nsc.Instance->OnCreate();
			});
		}
	}

	void Scene::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = false;

		// Invoke Actor.OnDestroy
		GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& scriptComponent)
		{
			Actor actor{ actorID, this };

			if (!actor.IsActive())
				return;

			actor.CallMethod(ScriptMethod::OnDestroy);
		});

		ScriptEngine::OnRuntimeStop();

		// C++ Actor Lifecycle
		GetAllActorsWith<NativeScriptComponent>().each([=](auto actorID, auto& nsc)
		{
			nsc.Instance->OnDestroy();
			nsc.DestroyInstanceScript(&nsc);
		});

		SystemManager::OnRuntimeStop(this);

		StopAnimatorsRuntime();
		OnPhysicsSimulationStop();

		m_Timers.clear();
	}

	void Scene::OnPhysicsSimulationStart()
	{
		VX_PROFILE_FUNCTION();

		m_IsSimulating = true;

		Physics::OnSimulationStart(this);
		Physics2D::OnSimulationStart(this);
	}

	void Scene::OnPhysicsSimulationUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		Physics::OnSimulationUpdate(delta);
		Physics2D::OnSimulationUpdate(delta, this);
	}

	void Scene::OnPhysicsSimulationStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsSimulating = false;

		Physics::OnSimulationStop(this);
		Physics2D::OnSimulationStop();
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		const bool updateCurrentFrame = !m_IsPaused || m_StepFrames > 0;

		if (updateCurrentFrame)
		{
#ifndef VX_DIST
			InstrumentationTimer timer("Scene::OnUpdateRuntime - Script Update");
#endif

			// Update C++ Actor
			GetAllActorsWith<NativeScriptComponent>().each([=](auto actorID, auto& nsc)
			{
				Actor actor{ actorID, this };
				
				if (!actor.IsActive())
					return;

				nsc.Instance->OnUpdate(delta);
			});

			// Invoke Actor.OnUpdate
			GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, this };

				if (!actor.IsActive())
					return;

				actor.CallMethod(ScriptMethod::OnUpdate);
			});

#ifndef VX_DIST
			Application& application = Application::Get();
			FrameTime& frameTime = application.GetFrameTime();
			frameTime.ScriptUpdateTime += timer.ElapsedMS();
#endif

#ifndef VX_DIST
			timer = InstrumentationTimer("Scene::OnUpdateRuntime - Physics Update");
#endif
			// Update Physics Bodies
			OnPhysicsSimulationUpdate(delta);

#ifndef VX_DIST
			frameTime.PhysicsUpdateTime += timer.ElapsedMS();
#endif

			// Update Animators
			OnAnimatorUpdateRuntime(delta);

			if (m_StepFrames)
			{
				m_StepFrames--;
			}
		}

		// Locate the scene's primary camera
		SceneCamera* primarySceneCamera = nullptr;
		TransformComponent primarySceneCameraTransform;

		// Render from the primary camera's point of view
		if (Actor primaryCameraActor = GetPrimaryCameraActor())
		{
			CameraComponent& cameraComponent = primaryCameraActor.GetComponent<CameraComponent>();
			primarySceneCamera = &cameraComponent.Camera;
			primarySceneCameraTransform = GetWorldSpaceTransform(primaryCameraActor);

			// Set clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);

			// Resize if needed
			if (primarySceneCamera->IsDirty())
			{
				ResizePrimaryCamera();
			}

			// Render
			SceneRenderPacket renderPacket{};
			renderPacket.PrimaryCamera = primarySceneCamera;
			renderPacket.PrimaryCameraViewMatrix = Math::Inverse(primarySceneCameraTransform.GetTransform());
			renderPacket.PrimaryCameraProjectionMatrix = primarySceneCamera->GetProjectionMatrix();
			renderPacket.PrimaryCameraWorldSpaceTranslation = primarySceneCameraTransform.Translation;
			renderPacket.TargetFramebuffer = m_TargetFramebuffer;
			renderPacket.Scene = this;
			renderPacket.IsEditorScene = false;
			s_SceneRenderer.RenderScene(renderPacket);

			// we need to set the original line width incase a script set it to something else
			SharedReference<Project> project = Project::GetActive();
			const ProjectProperties& properties = project->GetProperties();
			Renderer2D::SetLineWidth(properties.RendererProps.LineWidth);
		}

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		if (updateCurrentFrame)
		{
			OnUpdateActorTimers(delta);

			// Invoke Actor.OnPostUpdate
			GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, this };

				if (!actor.IsActive())
					return;

				actor.CallMethod(ScriptMethod::OnPostUpdate);
			});
		}

		ExecutePostUpdateQueue();
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		const bool updateCurrentFrame = !m_IsPaused || m_StepFrames > 0;

		if (updateCurrentFrame)
		{
			// Update Physics
			OnPhysicsSimulationUpdate(delta);

			// Update Animators
			OnAnimatorUpdateRuntime(delta);

			if (m_StepFrames)
			{
				m_StepFrames--;
			}
		}

		if (Actor primaryCameraActor = GetPrimaryCameraActor())
		{
			const CameraComponent& cameraComponent = primaryCameraActor.GetComponent<CameraComponent>();
			const SceneCamera& sceneCamera = cameraComponent.Camera;

			// Set Clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);

			// Resize if needed
			if (sceneCamera.IsDirty())
			{
				ResizePrimaryCamera();
			}
		}

		// Render
		{
			SceneRenderPacket renderPacket{};
			renderPacket.PrimaryCamera = camera;
			renderPacket.PrimaryCameraViewMatrix = camera->GetViewMatrix();
			renderPacket.PrimaryCameraProjectionMatrix = camera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_TargetFramebuffer;
			renderPacket.Scene = this;
			renderPacket.IsEditorScene = true;
			s_SceneRenderer.RenderScene(renderPacket);
		}

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		ExecutePostUpdateQueue();
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		// Update Animators
		OnAnimatorUpdateRuntime(delta);

		if (Actor primaryCameraActor = GetPrimaryCameraActor())
		{
			const CameraComponent& cameraComponent = primaryCameraActor.GetComponent<CameraComponent>();
			const SceneCamera& sceneCamera = cameraComponent.Camera;

			// Set Clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);

			// Resize if needed
			if (sceneCamera.IsDirty())
			{
				ResizePrimaryCamera();
			}
		}

		// Render
		{
			SceneRenderPacket renderPacket{};
			renderPacket.PrimaryCamera = camera;
			renderPacket.PrimaryCameraViewMatrix = camera->GetViewMatrix();
			renderPacket.PrimaryCameraProjectionMatrix = camera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_TargetFramebuffer;
			renderPacket.Scene = this;
			renderPacket.IsEditorScene = true;
			s_SceneRenderer.RenderScene(renderPacket);
		}

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		ExecutePostUpdateQueue();
	}

	void Scene::SubmitToPreUpdateQueue(const std::function<void()>& fn)
	{
		std::scoped_lock<std::mutex> lock(m_PreUpdateQueueMutex);

		m_PreUpdateQueue.emplace_back(fn);
	}

	void Scene::SubmitToPostUpdateQueue(const std::function<void()>& fn)
	{
		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		m_PostUpdateQueue.emplace_back(fn);
	}

	void Scene::InvokeActorOnGuiRender()
	{
		VX_PROFILE_FUNCTION();

		if (!m_IsRunning)
			return;

		// Invoke Actor.OnGuiRender
		GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
		{
			Actor actor{ actorID, this };

			if (!actor.IsActive())
				return;

			actor.CallMethod(ScriptMethod::OnGuiRender);
		});
	}

	void Scene::SetPaused(bool paused)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(m_IsRunning, "Scene must be running!");

		const bool consistent = m_IsPaused == paused;
		if (consistent)
			return;

		m_IsPaused = paused;

		if (m_IsPaused)
		{
			SystemManager::OnRuntimeScenePaused(this);

			GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, this };

				if (!actor.IsActive())
					return;

				actor.CallMethod(ScriptMethod::OnApplicationPause);
			});
		}
		else
		{
			SystemManager::OnRuntimeSceneResumed(this);

			GetAllActorsWith<ScriptComponent>().each([=](auto actorID, auto& sc)
			{
				Actor actor{ actorID, this };

				if (!actor.IsActive())
					return;

				actor.CallMethod(ScriptMethod::OnApplicationResume);
			});
		}
	}

	size_t Scene::GetScriptActorCount()
	{
		auto view = GetAllActorsWith<ScriptComponent>();

		return view.size();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		VX_PROFILE_FUNCTION();

		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize all non-FixedAspectRatio cameras
		auto view = GetAllActorsWith<CameraComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

			if (cameraComponent.FixedAspectRatio)
				continue;

			SceneCamera& camera = cameraComponent.Camera;
			camera.SetViewportSize(width, height);
			camera.SetDirty(false);
		}
	}

    const Timer& Scene::TryGetTimerByName(Actor actor, const std::string& name)
    {
		return (const Timer&)TryGetMutableTimerByName(actor, name);
    }

	Timer& Scene::TryGetMutableTimerByName(Actor actor, const std::string& name)
	{
		std::vector<Timer>& timers = m_Timers[actor];
		const size_t timerCount = timers.size();

		for (size_t i = 0; i < timerCount; i++)
		{
			Timer& timer = timers[i];
			const std::string& timerName = timer.GetName();
			if (String::FastCompare(timerName, name) == 0)
				continue;

			return timer;
		}

		return s_NullTimer;
	}

	void Scene::EmplaceOrReplaceTimer(Actor actor, Timer&& timer)
	{
		if (Timer& existing = TryGetMutableTimerByName(actor, timer.GetName()); existing != s_NullTimer)
		{
			existing = std::move(timer);
		}
		else
		{
			std::vector<Timer>& timers = m_Timers[actor];
			timers.push_back(timer);
		}
	}

	void Scene::ParentActor(Actor actor, Actor parent)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");
		VX_CORE_ASSERT(parent, "Parent was invalid!");

		if (parent.IsDescendantOf(actor))
		{
			UnparentActor(parent);

			if (Actor newParent = actor.GetParent())
			{
				UnparentActor(actor);
				ParentActor(parent, newParent);
			}
		}
		else if (Actor previousParent = actor.GetParent())
		{
			UnparentActor(actor);
		}

		if (parent) {
			actor.SetParentUUID(parent.GetUUID());
			parent.AddChild(actor.GetUUID());
		}

		ConvertToLocalSpace(actor);
	}

	void Scene::UnparentActor(Actor actor, bool convertToWorldSpace)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");

		Actor parent = actor.GetParent();
		VX_CORE_ASSERT(parent, "Parent was invalid!");
		if (!parent)
		{
			return;
		}

		std::vector<UUID>& parentChildren = parent.Children();
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), actor.GetUUID()), parentChildren.end());

		if (convertToWorldSpace)
		{
			ConvertToWorldSpace(actor);
		}

		actor.SetParentUUID(0);
	}

	void Scene::ActiveateChildren(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");

		const std::vector<UUID>& children = actor.Children();

		for (UUID uuid : children)
		{
			Actor child = TryGetActorWithUUID(uuid);

			if (!child)
				continue;

			child.SetActive(true);

			if (child.Children().size() > 0)
			{
				ActiveateChildren(child);
			}
		}
	}

	void Scene::DeactiveateChildren(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");

		const std::vector<UUID>& children = actor.Children();

		for (UUID uuid : children)
		{
			Actor child = TryGetActorWithUUID(uuid);

			if (!child)
				continue;

			child.SetActive(false);

			if (child.Children().size() > 0)
			{
				DeactiveateChildren(child);
			}
		}
	}

	Actor Scene::GetRootActorInHierarchy(Actor child) const
	{
		VX_PROFILE_FUNCTION();

		if (!child.HasParent())
		{
			return child;
		}

		Actor parent = child.GetParent();
		return GetRootActorInHierarchy(parent);
	}

	Actor Scene::GetPrimaryCameraActor()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<CameraComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			const CameraComponent& cc = actor.GetComponent<CameraComponent>();

			if (!actor.IsActive())
				continue;

			if (!cc.Primary)
				continue;
			
			return actor;
		}

		return Actor{};
	}

	Actor Scene::GetEnvironmentActor()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<SkyboxComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };

			if (!actor.IsActive())
				continue;

			return actor;
		}

		return Actor{};
	}

	Actor Scene::GetSkyLightActor()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<LightSourceComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			const LightSourceComponent& lsc = actor.GetComponent<LightSourceComponent>();

			if (!actor.IsActive())
				continue;

			if (lsc.Type != LightType::Directional)
				continue;

			return actor;
		}

		return Actor{};
	}

	Actor Scene::TryGetActorWithUUID(UUID uuid) const
	{
		VX_PROFILE_FUNCTION();

		if (!m_ActorMap.contains(uuid))
		{
			return Actor{};
		}

		return m_ActorMap.at(uuid);
	}

	Actor Scene::FindActorByName(std::string_view name)
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<TagComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			const std::string& tag = actor.Name();
			if (String::FastCompare(name, tag) == 0)
				continue;

			return Actor{ actor, this };
		}

		return Actor{};
	}

	Actor Scene::FindActorByID(entt::entity actorID)
	{
		VX_PROFILE_FUNCTION();

		m_Registry.each([&](auto id)
		{
			if (id == actorID)
			{
				return Actor{ id, this };
			}
		});

		return Actor{};
	}

	void Scene::ConvertToLocalSpace(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Actor parent = TryGetActorWithUUID(actor.GetParentUUID());

		if (!parent)
		{
			return;
		}

		TransformComponent& transform = actor.GetTransform();
		Math::mat4 parentTransform = GetWorldSpaceTransformMatrix(parent);
		Math::mat4 localTransform = Math::Inverse(parentTransform) * transform.GetTransform();
		transform.SetTransform(localTransform);
	}

	void Scene::ConvertToWorldSpace(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Actor parent = TryGetActorWithUUID(actor.GetParentUUID());

		if (!parent)
		{
			return;
		}

		Math::mat4 transform = GetWorldSpaceTransformMatrix(actor);
		TransformComponent& actorTransform = actor.GetTransform();
		actorTransform.SetTransform(transform);
	}

	Math::mat4 Scene::GetWorldSpaceTransformMatrix(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 transform(1.0f);

		UUID parentUUID = actor.GetParentUUID();
		Actor parent = TryGetActorWithUUID(parentUUID);

		if (parent)
		{
			transform = GetWorldSpaceTransformMatrix(parent);
		}

		return transform * actor.GetTransform().GetTransform();
	}

	TransformComponent Scene::GetWorldSpaceTransform(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 transform = GetWorldSpaceTransformMatrix(actor);
		TransformComponent transformComponent;
		transformComponent.SetTransform(transform);
		return transformComponent;
	}

	// This is clearly a design flaw with the renderer, it should already have all of this data but yet we
	// still need to go and gather it ourselves which is inefficient
    SharedReference<SceneGeometry>& Scene::GetSceneMeshes()
	{
		if (m_SceneMeshes == nullptr)
		{
			m_SceneMeshes = SharedReference<SceneGeometry>::Create();
		}

		ClearSceneMeshes();

		auto meshView = GetAllActorsWith<MeshRendererComponent>();

		for (const auto meshRenderer : meshView)
		{
			Actor actor{ meshRenderer, this };
			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();

			// Skip if not active
			if (!actor.IsActive())
				continue;

			if (!meshRendererComponent.Visible)
				continue;

			if (!meshRendererComponent.CastShadows)
				continue;

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
				continue;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				continue;

			m_SceneMeshes->MeshEntities.push_back(actor);

			m_SceneMeshes->Meshes.push_back(mesh);

			const Math::mat4 worldSpaceTransform = GetWorldSpaceTransformMatrix(actor);
			m_SceneMeshes->WorldSpaceMeshTransforms.push_back(worldSpaceTransform);
		}

		auto staticMeshView = GetAllActorsWith<StaticMeshRendererComponent>();

		for (const auto staticMeshRenderer : staticMeshView)
		{
			Actor actor{ staticMeshRenderer, this };
			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();

			// Skip if not active
			if (!actor.IsActive())
				continue;

			if (!staticMeshRendererComponent.Visible)
				continue;

			if (!staticMeshRendererComponent.CastShadows)
				continue;

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				continue;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				continue;

			m_SceneMeshes->StaticMeshes.push_back(staticMesh);

			const Math::mat4 worldSpaceTransform = GetWorldSpaceTransformMatrix(actor);
			m_SceneMeshes->WorldSpaceStaticMeshTransforms.push_back(worldSpaceTransform);
		}

		return m_SceneMeshes;
	}

	void Scene::SortActors()
	{
		VX_PROFILE_FUNCTION();

		m_Registry.sort<IDComponent>([&](const auto lhs, const auto rhs)
		{
			auto lhsActor = m_ActorMap.find(lhs.ID);
			auto rhsActor = m_ActorMap.find(rhs.ID);
			return static_cast<uint32_t>(lhsActor->second) < static_cast<uint32_t>(rhsActor->second);
		});
	}

	void Scene::ResizePrimaryCamera()
	{
		Actor primaryCameraActor = GetPrimaryCameraActor();
		if (!primaryCameraActor)
		{
			return;
		}

		CameraComponent& cameraComponent = primaryCameraActor.GetComponent<CameraComponent>();
		SceneCamera& sceneCamera = cameraComponent.Camera;
		sceneCamera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		sceneCamera.SetDirty(false);
	}

	void Scene::StopAnimatorsRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<TransformComponent, AnimatorComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			SharedRef<Animator> animator = actor.GetComponent<AnimatorComponent>().Animator;

			if (!animator->IsPlaying())
				continue;

			animator->Stop();
		}
	}

	void Scene::OnMeshUpdateRuntime()
	{
		VX_PROFILE_FUNCTION();

		{
			auto view = GetAllActorsWith<MeshRendererComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, this };
				MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();

				AssetHandle meshHandle = meshRendererComponent.Mesh;
				if (!AssetManager::IsHandleValid(meshHandle))
					continue;

				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
				if (!mesh)
					continue;

				mesh->OnUpdate((int)(entt::entity)e);
			}
		}

		{
			auto view = GetAllActorsWith<StaticMeshRendererComponent>();

			for (const auto e : view)
			{
				Actor actor{ e, this };
				StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();

				AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
				if (!AssetManager::IsHandleValid(staticMeshHandle))
					continue;

				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
				if (!staticMesh)
					continue;

				SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;
				if (!materialTable)
					continue;

				staticMesh->OnUpdate(materialTable, (int)(entt::entity)e);
			}
		}
	}

	void Scene::OnAnimatorUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllActorsWith<AnimatorComponent, AnimationComponent, MeshRendererComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, this };
			SharedRef<Animator> animator = actor.GetComponent<AnimatorComponent>().Animator;

			if (!animator)
				continue;

			if (!animator->IsPlaying())
				continue;

			animator->UpdateAnimation(delta);
		}
	}

	void Scene::ClearSceneMeshes()
	{
		m_SceneMeshes->MeshEntities.clear();
		m_SceneMeshes->Meshes.clear();
		m_SceneMeshes->WorldSpaceMeshTransforms.clear();
		m_SceneMeshes->StaticMeshes.clear();
		m_SceneMeshes->WorldSpaceStaticMeshTransforms.clear();
	}

	void Scene::ExecutePreUpdateQueue()
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PreUpdateQueueMutex);

		for (const auto& fn : m_PreUpdateQueue)
		{
			std::invoke(fn);
		}

		m_PreUpdateQueue.clear();
	}

	void Scene::ExecutePostUpdateQueue()
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		for (const auto& fn : m_PostUpdateQueue)
		{
			std::invoke(fn);
		}

		m_PostUpdateQueue.clear();
	}

	void Scene::OnComponentUpdate(TimeStep delta)
	{
		OnMeshUpdateRuntime();
	}

	void Scene::OnSystemUpdate(TimeStep delta)
	{
		SystemManager::GetAssetSystem<ParticleSystem>()->OnUpdateRuntime(this, delta);
		SystemManager::GetSystem<UISystem>()->OnUpdateRuntime(this);
	}

	void Scene::DestroyActorInternal(Actor actor, bool excludeChildren)
	{
		VX_PROFILE_FUNCTION();

#ifdef VX_DEBUG
		const uint32_t garbage = 0xcccccccc;
		uint32_t addr = (uint32_t)actor.GetContextScene();
		if (addr == garbage)
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyActorInternal with invalid Scene!");
			return;
		}
		addr = (uint32_t)actor.operator entt::entity();
		if (addr == garbage)
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyActorInternal with invalid Actor!");
			return;
		}
#endif

		if (!actor || !m_ActorMap.contains(actor))
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyActorInternal with invalid Actor!");
			VX_CORE_ASSERT(false, "Trying to free invalid Actor!");
			return;
		}

		if (Actor selected = SelectionManager::GetSelectedActor(); selected == actor)
		{
			SelectionManager::DeselectActor();
		}

		if (m_IsRunning)
		{
			// Invoke Actor.OnDestroy
			actor.CallMethod(ScriptMethod::OnDestroy);

			if (actor.HasComponent<NativeScriptComponent>())
			{
				NativeScriptComponent& nsc = actor.GetComponent<NativeScriptComponent>();
				nsc.Instance->OnDestroy();
				nsc.DestroyInstanceScript(&nsc);
			}

			// Destroy physics body
			Physics::DestroyPhysicsActor(actor);
			Physics2D::DestroyPhysicsBody(actor);
		}

		if (Actor parent = actor.GetParent())
		{
			parent.RemoveChild(actor.GetUUID());
		}

		if (!excludeChildren)
		{
			for (size_t i = 0; i < actor.Children().size(); i++)
			{
				UUID childID = actor.Children()[i];
				Actor child = TryGetActorWithUUID(childID);
				DestroyActorInternal(child, excludeChildren);
			}
		}

		auto it = m_ActorMap.find(actor.GetUUID());
		VX_CORE_ASSERT(it != m_ActorMap.end(), "Enitiy was not found in Actor Map!");

		if (it == m_ActorMap.end())
		{
			VX_CONSOLE_LOG_ERROR("Actor was not found in Actor Map!");
			return;
		}

		// Remove the actor from our internal map
		m_ActorMap.erase(it->first);
		m_Registry.destroy(actor);

		SortActors();
	}

	void Scene::OnUpdateActorTimers(TimeStep delta)
	{
		for (auto& [actor, timers] : m_Timers)
		{
			// update all the timers for each actor
			for (Timer& timer : timers)
			{
				if (timer.IsFinished())
				{
					m_FinishedTimers.push_back(timer);
					continue;
				}

				timer.OnUpdate(delta);
			}

			// remove finished timers
			for (Timer& timer : m_FinishedTimers)
			{
				const std::string& timerName = timer.GetName();
				const size_t timerCount = timers.size();

				for (size_t pos = 0; pos < timerCount; pos++)
				{
					const std::string& potential = timers[pos].GetName();

					if (!String::FastCompare(timerName, potential))
						continue;

					timers.erase(timers.begin() + pos);
					break;
				}
			}

			m_FinishedTimers.clear();
		}
	}

	void Scene::OnCameraConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();

		if (m_ViewportWidth == 0 || m_ViewportHeight == 0)
		{
			return;
		}

		cameraComponent.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		if (cameraComponent.Primary)
		{
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
		}
	}

	void Scene::OnStaticMeshConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		StaticMeshRendererComponent& staticMeshComponent = actor.GetComponent<StaticMeshRendererComponent>();

		if (staticMeshComponent.Type != MeshType::Custom)
		{
			DefaultMesh::StaticMeshType staticMeshType = (DefaultMesh::StaticMeshType)staticMeshComponent.Type;

			staticMeshComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(staticMeshType);

			SharedReference<MaterialTable> materialTable = staticMeshComponent.Materials;

			if (AssetManager::IsHandleValid(staticMeshComponent.StaticMesh) && materialTable->Empty())
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshComponent.StaticMesh);
				staticMesh->LoadMaterialTable(materialTable);
			}
		}
	}

	void Scene::OnParticleEmitterConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		ParticleEmitterComponent& pmc = actor.GetComponent<ParticleEmitterComponent>();

		if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
		{
			pmc.EmitterHandle = AssetManager::CreateMemoryOnlyAsset<ParticleEmitter>();
		}
	}

	void Scene::OnParticleEmitterDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		SystemManager::GetAssetSystem<ParticleSystem>()->DestroyAsset(actor);
	}

	void Scene::OnTextMeshConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();

		if (!AssetManager::IsHandleValid(textMeshComponent.FontAsset))
		{
			textMeshComponent.FontAsset = Font::GetDefaultFont()->Handle;
		}
	}

	void Scene::OnAnimatorConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();

		if (actor.HasComponent<AnimationComponent>())
		{
			const AnimationComponent& animationComponent = actor.GetComponent<AnimationComponent>();
			animatorComponent.Animator = Animator::Create(animationComponent.Animation);
		}
	}

	void Scene::OnAnimationConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		const AnimationComponent& animationComponent = actor.GetComponent<AnimationComponent>();

		if (actor.HasComponent<MeshRendererComponent>())
		{
			auto& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			AssetHandle meshHandle = meshRendererComponent.Mesh;

			if (AssetManager::IsHandleValid(meshHandle))
			{
				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);

				if (mesh)
				{
					// TODO fix this
					//component.Animation = Animation::Create(filepath, mesh->Handle);
				}
			}
		}
	}

	void Scene::OnAudioSourceConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
		
		if (!AssetManager::IsHandleValid(asc.AudioHandle))
		{
			asc.AudioHandle = AssetManager::CreateMemoryOnlyAsset<AudioSource>();
		}
	}

	void Scene::OnAudioSourceDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		// TODO do we need to do anything here?
	}

	void Scene::OnAudioListenerConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		const AudioListenerComponent& alc = actor.GetComponent<AudioListenerComponent>();
		if (!AssetManager::IsHandleValid(alc.ListenerHandle))
		{
			// TODO
		}
	}

	void Scene::OnAudioListenerDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Actor actor = { e, this };
		// TODO
	}

	SharedReference<Scene> Scene::Copy(SharedReference<Scene>& source)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Scene> destination = Scene::Create(source->m_TargetFramebuffer);

		destination->m_TargetFramebuffer = source->m_TargetFramebuffer;
		destination->m_ViewportWidth = source->m_ViewportWidth;
		destination->m_ViewportHeight = source->m_ViewportHeight;

		entt::registry& srcSceneRegistry = source->m_Registry;
		entt::registry& dstSceneRegistry = destination->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entites in new scene
		auto view = srcSceneRegistry.view<IDComponent>();
		for (const auto e : view)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const TagComponent& tagComponent = srcSceneRegistry.get<TagComponent>(e);
			const std::string& name = tagComponent.Tag;
			const std::string& marker = tagComponent.Marker;
			Actor copiedActor = destination->CreateActorWithUUID(uuid, name, marker);
			copiedActor.SetActive(tagComponent.IsActive);
			enttMap[uuid] = (entt::entity)copiedActor;
		}

		// Copy all components (except IDComponent and TagComponent)
		ComponentUtils::CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		destination->SortActors();

		return destination;
	}

	void Scene::CreateSampleScene(ProjectType type, SharedReference<Scene>& context)
	{
		switch (type)
		{
			case ProjectType::e2D:
			{
				// Starting Entities
				Actor sprite = context->CreateActor("Sprite");
				sprite.AddComponent<SpriteRendererComponent>();
				sprite.AddComponent<RigidBodyComponent>();
				sprite.AddComponent<BoxColliderComponent>();

				Actor primaryCamera = context->CreateActor("Camera");
				// we can do this once audio is fixed
				//startingCamera.AddComponent<AudioListenerComponent>();
				SceneCamera& camera = primaryCamera.AddComponent<CameraComponent>().Camera;
				camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				TransformComponent& cameraTransform = primaryCamera.GetTransform();
				cameraTransform.Translation = { 0.0f, 0.0f, 0.0f };
				cameraTransform.SetRotationEuler({ 0.0f, 0.0f, 0.0f });
				break;
			}
			case ProjectType::e3D:
			{
				// Starting Entities
				Actor cube = context->CreateActor("Cube");
				cube.AddComponent<StaticMeshRendererComponent>();
				cube.AddComponent<RigidBodyComponent>();
				cube.AddComponent<BoxColliderComponent>();

				Actor skylight = context->CreateActor("Sky Light");
				LightSourceComponent& lsc = skylight.AddComponent<LightSourceComponent>();
				lsc.Type = LightType::Directional;
				lsc.ShadowBias = 0.0f;
				skylight.GetTransform().SetRotationEuler({ 0.0f, Math::Deg2Rad(-57.0f), 0.0f });
				skylight.GetTransform().Translation = { -1.0f, 5.0f, 1.0f };

				Actor primaryCamera = context->CreateActor("Primary Camera");
				// ditto
				//startingCamera.AddComponent<AudioListenerComponent>();
				SceneCamera& camera = primaryCamera.AddComponent<CameraComponent>().Camera;
				camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				TransformComponent& cameraTransform = primaryCamera.GetTransform();
				cameraTransform.Translation = { -4.0f, 3.0f, 4.0f };
				cameraTransform.SetRotationEuler({ Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f });
				break;
			}
		}
	}

	SharedReference<Scene> Scene::Create(SharedReference<Framebuffer> targetFramebuffer)
	{
		return SharedReference<Scene>::Create(targetFramebuffer);
	}

	SharedReference<Scene> Scene::Create()
	{
		return SharedReference<Scene>::Create();
	}

}
