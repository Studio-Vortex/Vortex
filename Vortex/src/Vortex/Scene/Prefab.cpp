#include "vxpch.h"
#include "Prefab.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/Serialization/SceneSerializer.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	Prefab::Prefab(const std::filesystem::path& filepath)
	{
		m_Scene = Scene::Create();
		m_Actor = Actor{};
		m_Filepath = filepath;
	}

	Prefab::Prefab(Actor actor)
	{
		m_Scene = Scene::Create();
		m_Actor = CreatePrefabFromActor(actor);
	}

	Actor Prefab::CreatePrefabFromActor(Actor actor)
	{
		Actor newActor = m_Scene->CreateActor();

		// Add prefab component
		PrefabComponent& prefabComponent = newActor.AddComponent<PrefabComponent>();
		prefabComponent.PrefabUUID = UUID();
		prefabComponent.ActorUUID = actor.GetUUID();

		actor.m_Scene->CopyComponentIfExists<TagComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<TransformComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CameraComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SkyboxComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<LightSourceComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<MeshRendererComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<StaticMeshRendererComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SpriteRendererComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CircleRendererComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<ParticleEmitterComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<TextMeshComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<AudioSourceComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<AudioListenerComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<RigidBodyComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CharacterControllerComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<FixedJointComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<BoxColliderComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SphereColliderComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CapsuleColliderComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<MeshColliderComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<RigidBody2DComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<BoxCollider2DComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CircleCollider2DComponent>(newActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<ScriptComponent>(newActor, m_Scene->m_Registry, actor);

		const auto& children = actor.Children();
		for (const auto& childID : children)
		{
			Actor sourceChild = actor.m_Scene->TryGetActorWithUUID(childID);
			VX_CORE_ASSERT(sourceChild, "Child ID Invalid --- This should never happen");
			Actor childDuplicate = CreatePrefabFromActor(sourceChild);

			childDuplicate.SetParentUUID(newActor.GetUUID());
			newActor.Children().push_back(childDuplicate.GetUUID());
		}

		if (newActor.HasComponent<ScriptComponent>())
		{
			ScriptEngine::RuntimeInstantiateActor(newActor);
		}

		return newActor;
	}

	SharedRef<Prefab> Prefab::Create(const std::filesystem::path& filepath)
	{
		return CreateShared<Prefab>(filepath);
	}

	SharedRef<Prefab> Prefab::Create(Actor actor)
	{
		return CreateShared<Prefab>(actor);
	}

}
