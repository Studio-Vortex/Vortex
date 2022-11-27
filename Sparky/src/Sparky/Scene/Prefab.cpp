#include "sppch.h"
#include "Prefab.h"

#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/SceneSerializer.h"
#include "Sparky/Scripting/ScriptEngine.h"

namespace Sparky {

	Prefab::Prefab(const std::filesystem::path& filepath)
	{
		m_Scene = Scene::Create();
		m_Entity = Entity{};
		m_Filepath = filepath;
	}

	Prefab::Prefab(Entity entity)
	{
		m_Scene = Scene::Create();
		m_Entity = CreatePrefabFromEntity(entity);
	}

	Prefab::~Prefab()
	{

	}

	Entity Prefab::CreatePrefabFromEntity(Entity entity, bool serialize)
	{
		Entity newEntity = m_Scene->CreateEntity();

		// Add prefab component
		PrefabComponent& prefabComponent = newEntity.AddComponent<PrefabComponent>();
		prefabComponent.PrefabUUID = UUID();
		prefabComponent.EntityUUID = entity.GetUUID();

		entity.m_Scene->CopyComponentIfExists<TagComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<TransformComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<CameraComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<SkyboxComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<LightSourceComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<MeshRendererComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<SpriteRendererComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<CircleRendererComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<ParticleEmitterComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<TextMeshComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<AudioSourceComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<AudioListenerComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<RigidBodyComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<CharacterControllerComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<PhysicsMaterialComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<BoxColliderComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<SphereColliderComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<CapsuleColliderComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<StaticMeshColliderComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<RigidBody2DComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<BoxCollider2DComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<CircleCollider2DComponent>(newEntity, m_Scene->m_Registry, entity);
		entity.m_Scene->CopyComponentIfExists<ScriptComponent>(newEntity, m_Scene->m_Registry, entity);

		const auto& children = entity.Children();
		for (const auto& childID : children)
		{
			Entity sourceChild = entity.m_Scene->TryGetEntityWithUUID(childID);
			SP_CORE_ASSERT(sourceChild, "Child ID Invalid --- This should never happen");
			Entity childDuplicate = CreatePrefabFromEntity(sourceChild);

			childDuplicate.SetParent(newEntity.GetUUID());
			newEntity.Children().push_back(childDuplicate.GetUUID());
		}

		if (newEntity.HasComponent<ScriptComponent>())
		{
			ScriptEngine::DuplicateScriptInstance(entity, newEntity);
		}

		return newEntity;
	}

	SharedRef<Prefab> Prefab::Create(const std::filesystem::path& filepath)
	{
		return CreateShared<Prefab>(filepath);
	}

	SharedRef<Prefab> Prefab::Create(Entity entity)
	{
		return CreateShared<Prefab>(entity);
	}

}
