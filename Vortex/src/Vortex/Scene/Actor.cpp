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
			OnEnable();
		}
		else
		{
			OnDisable();
		}
	}

	bool Actor::CallMethod(ManagedMethod method)
	{
		const Actor self = *this;

		if (!ScriptEngine::HasValidScriptClass(self))
			return false;

		if (!ScriptEngine::ScriptInstanceHasMethod(self, method))
			return false;

		return ScriptEngine::Invoke(method, self);
	}

	void Actor::OnEnable()
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
			self.CallMethod(ManagedMethod::OnEnable);
		}

		// create the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
		{
			Physics::CreatePhysicsActor(self);
		}
	}

	void Actor::OnDisable()
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
			self.CallMethod(ManagedMethod::OnDisable);
		}

		// destroy the rigid body if the actor has one
		if (sceneSimulating && isPhysicsActor)
		{
			Physics::DestroyPhysicsActor(self);
		}
	}

}
