#include "vxpch.h"
#include "Prefab.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Asset/AssetImporter.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	Prefab::Prefab()
	{
		m_Scene = Scene::Create();
	}

	void Prefab::Create(Actor actor, bool serialize)
	{
		m_Scene = Scene::Create();
		m_Actor = CreatePrefabFromActor(actor);

		if (serialize)
		{
			AssetImporter::Serialize(SharedReference<Prefab>(this));
		}
	}

	Actor Prefab::CreatePrefabFromActor(Actor actor)
	{
		VX_CORE_ASSERT(Handle, "invalid asset handle!");

		Actor prefabActor = m_Scene->CreateActor();

		// Add prefab component
		PrefabComponent& prefabComponent = prefabActor.AddComponent<PrefabComponent>();
		prefabComponent.Prefab = Handle;
		prefabComponent.ActorUUID = prefabActor.GetUUID();

		actor.m_Scene->CopyComponentIfExists<TagComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<TransformComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CameraComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SkyboxComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<LightSourceComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<MeshRendererComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<StaticMeshRendererComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SpriteRendererComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CircleRendererComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<ParticleEmitterComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<TextMeshComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<ButtonComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<AudioSourceComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<AudioListenerComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<RigidBodyComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CharacterControllerComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<FixedJointComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<BoxColliderComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<SphereColliderComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CapsuleColliderComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<MeshColliderComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<RigidBody2DComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<BoxCollider2DComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<CircleCollider2DComponent>(prefabActor, m_Scene->m_Registry, actor);
		actor.m_Scene->CopyComponentIfExists<ScriptComponent>(prefabActor, m_Scene->m_Registry, actor);

		const std::vector<UUID>& children = actor.Children();
		for (const UUID& childID : children)
		{
			Actor childDuplicate = CreatePrefabFromActor(actor.m_Scene->TryGetActorWithUUID(childID));

			childDuplicate.SetParentUUID(prefabActor.GetUUID());
			prefabActor.AddChild(childDuplicate.GetUUID());
		}

		if (prefabActor.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(actor, prefabActor);
		}

		return prefabActor;
	}

}
