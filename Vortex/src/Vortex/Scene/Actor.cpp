#include "vxpch.h"
#include "Actor.h"

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

	void Actor::RemoveChild(UUID childUUID) const
	{
		std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;
		auto it = std::find(children.begin(), children.end(), childUUID);
		VX_CORE_ASSERT(it != children.end(), "Child UUID was not found");
		if (it == children.end())
		{
			return;
		}
		children.erase(it);
	}

	bool Actor::IsAncesterOf(Actor actor) const
	{
		const std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;

		if (children.empty())
			return false;

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
		const bool isPhysicsActor = Physics::IsPhysicsActor(actor->GetUUID());

		context->ActiveateChildren(*actor);

		if (!actor->HasComponent<ScriptComponent>())
		{
			return;
		}

		if (sceneRunning && actor->HasComponent<ScriptComponent>())
		{
			const ScriptComponent& scriptComponent = actor->GetComponent<ScriptComponent>();
			if (scriptComponent.Enabled && !scriptComponent.Instantiated)
			{
				ScriptEngine::RT_InstantiateActor(*actor);
				context->ActiveateChildren(*actor);
				return;
			}
		}

		// Invoke Actor.OnEnable
		if (sceneRunning)
		{
			actor->CallMethod(ScriptMethod::OnEnable);
		}

		// create the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
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
		const bool isPhysicsActor = Physics::IsPhysicsActor(actor->GetUUID());

		context->DeactiveateChildren(*actor);

		if (!actor->HasComponent<ScriptComponent>())
		{
			return;
		}

		// Invoke Actor.OnDisable
		if (sceneRunning)
		{
			actor->CallMethod(ScriptMethod::OnDisable);
		}

		// destroy the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
		{
			Physics::DestroyPhysicsActor(*actor);
		}
	}

}
