#include "vxpch.h"
#include "Actor.h"

#include "Vortex/Physics/3D/Physics.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void Actor::SetActive(bool active) const
	{
		VX_PROFILE_FUNCTION();

		if (active)
		{
			OnEnabled();
		}
		else
		{
			OnDisabled();
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

		if (children.size() == 0)
			return false;

		for (UUID child : children)
		{
			if (child == actor.GetUUID())
				return true;
		}

		for (UUID child : children)
		{
			if (m_Scene->TryGetActorWithUUID(child).IsAncesterOf(actor))
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

	void Actor::OnEnabled() const
	{
		GetComponent<TagComponent>().IsActive = true;

		Actor self = *this;
		Scene* context = self.GetContextScene();

		const bool sceneRunning = context->IsRunning();
		const bool sceneSimulating = sceneRunning || context->IsSimulating();
		const bool isPhysicsActor = Physics::IsPhysicsActor(self.GetUUID());

		context->ActiveateChildren(self);

		// Invoke Actor.OnEnable
		if (sceneRunning)
		{
			self.CallMethod(ScriptMethod::OnEnable);
		}

		// create the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
		{
			Physics::CreatePhysicsActor(self);
		}
	}

	void Actor::OnDisabled() const
	{
		GetComponent<TagComponent>().IsActive = false;

		Actor self = *this;
		Scene* context = self.GetContextScene();

		const bool sceneRunning = context->IsRunning();
		const bool sceneSimulating = sceneRunning || context->IsSimulating();
		const bool isPhysicsActor = Physics::IsPhysicsActor(self.GetUUID());

		context->DeactiveateChildren(self);

		// Invoke Actor.OnDisable
		if (sceneRunning)
		{
			self.CallMethod(ScriptMethod::OnDisable);
		}

		// destroy the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
		{
			Physics::DestroyPhysicsActor(self);
		}
	}

}
