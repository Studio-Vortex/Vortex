#include "vxpch.h"
#include "Actor.h"

#include "Vortex/Scene/Scene.h"

#include "Vortex/Physics/3D/Physics.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Actor::SetActive(bool active)
	{
		VX_PROFILE_FUNCTION();

		if (active)
		{
			OnEnabled(this);
		}
		else
		{
			OnDisabled(this);
		}
	}

	bool Actor::IsActive() const
	{
		return GetComponent<TagComponent>().IsActive;
	}

	UUID Actor::GetParentUUID() const
	{
		return GetComponent<HierarchyComponent>().ParentUUID;
	}

    Actor Actor::GetParent() const
    {
		UUID parentUUID = GetParentUUID();
		return m_Scene->TryGetActorWithUUID(parentUUID);
    }

    void Actor::SetParentUUID(UUID parentUUID) const
	{
		GetComponent<HierarchyComponent>().ParentUUID = parentUUID;
	}

	std::vector<UUID>& Actor::Children()
	{
		return GetComponent<HierarchyComponent>().Children;
	}

	void Actor::AddChild(UUID childUUID) const
	{
		GetComponent<HierarchyComponent>().Children.push_back(childUUID);
	}

	bool Actor::HasChild(UUID childUUID) const
	{
		const std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;
		auto it = std::find(children.begin(), children.end(), childUUID);
		return it != children.end();
	}

	bool Actor::RemoveChild(UUID childUUID) const
	{
		std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;
		auto it = std::find(children.begin(), children.end(), childUUID);
		if (it == children.end()) {
			return false;
		}
		children.erase(it);
		return true;
	}

	bool Actor::IsAncesterOf(Actor actor) const
	{
		const std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;

		if (children.empty()) {
			return false;
		}

		for (UUID childUUID : children)
		{
			if (childUUID != actor.GetUUID())
				continue;

			return true;
		}

		for (UUID child : children)
		{
			if (!m_Scene->TryGetActorWithUUID(child).IsAncesterOf(actor))
				continue;

			return true;
		}

		return false;
	}

	bool Actor::CallMethod(ScriptMethod method, const std::vector<RuntimeMethodArgument>& argumentList) const
	{
		const Actor self = *this;

		if (!ScriptEngine::IsScriptClassValid(self))
			return false;

		if (!ScriptEngine::ScriptInstanceHasMethod(self, method))
			return false;

		if (!ScriptEngine::IsScriptComponentEnabled(self))
			return false;

		return ScriptEngine::Invoke(self, method, argumentList);
	}

	void Actor::OnEnabled(Actor* actor)
	{
		actor->GetComponent<TagComponent>().IsActive = true;

		Scene* context = actor->GetContextScene();

		const bool sceneRunning = context->IsRunning();
		const bool sceneSimulating = sceneRunning || context->IsSimulating();
		const bool isPhysicsActor = actor->HasComponent<RigidBodyComponent>();
		const bool rigidbodyCreated = Physics::IsPhysicsActor(actor->GetUUID());

		if (sceneRunning && actor->HasComponent<ScriptComponent>())
		{
			const ScriptComponent& scriptComponent = actor->GetComponent<ScriptComponent>();
			if (scriptComponent.Enabled && !scriptComponent.Instantiated)
			{
				ScriptEngine::RT_InstantiateActor(*actor);
			}
		}

		context->ActiveateChildren(*actor);

		// Invoke Actor.OnEnable
		if (sceneRunning)
		{
			actor->CallMethod(ScriptMethod::OnEnable);
		}

		// create the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor && !rigidbodyCreated)
		{
			Physics::CreatePhysicsActor(*actor);
		}
	}

	void Actor::OnDisabled(Actor* actor)
	{
		actor->GetComponent<TagComponent>().IsActive = false;

		Scene* context = actor->GetContextScene();

		const bool sceneRunning = context->IsRunning();
		const bool sceneSimulating = sceneRunning || context->IsSimulating();
		const bool isPhysicsActor = actor->HasComponent<RigidBodyComponent>();
		const bool rigidbodyCreated = Physics::IsPhysicsActor(actor->GetUUID());

		context->DeactiveateChildren(*actor);

		// Invoke Actor.OnDisable
		if (sceneRunning)
		{
			actor->CallMethod(ScriptMethod::OnDisable);
		}

		// destroy the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor && rigidbodyCreated)
		{
			Physics::DestroyPhysicsActor(*actor);
		}
	}

}
