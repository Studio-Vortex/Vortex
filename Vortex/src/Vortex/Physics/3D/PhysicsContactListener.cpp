#include "vxpch.h"
#include "PhysicsContactListener.h"

#include "Vortex/Physics/3D/Physics.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

namespace Vortex {

	namespace Utils {

		void CallMethod(Actor actor, ManagedMethod method, const std::vector<RuntimeMethodArgument>& argumentList = {})
		{
			if (!ScriptEngine::HasValidScriptClass(actor))
				return;

			if (!ScriptEngine::ScriptInstanceHasMethod(actor, method))
				return;

			ScriptEngine::Invoke(method, actor, argumentList);
		}

	}

	void PhysicsContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (!contextScene || !contextScene->IsRunning())
		{
			return;
		}

		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxJoint* nativeJoint = (physx::PxJoint*)constraints[i].externalReference;
			const ConstrainedJointData* jointData = (ConstrainedJointData*)nativeJoint->userData;

			nativeJoint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);

			Actor actor = contextScene->TryGetActorWithUUID(jointData->ActorUUID);

			if (!actor)
			{
				continue;
			}

			Actor connectedActor;

			if (actor.HasComponent<FixedJointComponent>())
			{
				const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
				UUID connectedActorUUID = fixedJointComponent.ConnectedActor;

				if (Actor attachedActor = contextScene->TryGetActorWithUUID(connectedActorUUID))
				{
					connectedActor = attachedActor;
				}
			}

			if (!connectedActor)
			{
				continue;
			}

			if (nativeJoint->is<physx::PxFixedJoint>())
			{
				physx::PxFixedJoint* fixedJoint = nativeJoint->is<physx::PxFixedJoint>();
				const std::pair<Math::vec3, Math::vec3>& forceAndTorque = Physics::GetLastReportedFixedJointForces(fixedJoint);

				RuntimeMethodArgument arg0(forceAndTorque);

				if (ScriptEngine::HasValidScriptClass(actor))
				{
					ScriptEngine::Invoke(ManagedMethod::OnFixedJointDisconnected, actor, { arg0 });
				}

				if (ScriptEngine::HasValidScriptClass(connectedActor))
				{
					ScriptEngine::Invoke(ManagedMethod::OnFixedJointDisconnected, connectedActor, { arg0 });
				}
			}
		}
	}

	void PhysicsContactListener::onWake(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void PhysicsContactListener::onSleep(physx::PxActor** actors, physx::PxU32 count)
	{
		PX_UNUSED(actors);
		PX_UNUSED(count);
	}

	void PhysicsContactListener::onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (!contextScene || !contextScene->IsRunning())
		{
			return;
		}

		const bool removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		const bool removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

		if (removedActorA || removedActorB)
		{
			return;
		}

		const PhysicsBodyData* actorA_UserData = (PhysicsBodyData*)pairHeader.actors[0]->userData;
		const PhysicsBodyData* actorB_UserData = (PhysicsBodyData*)pairHeader.actors[1]->userData;

		if (!actorA_UserData || !actorB_UserData)
		{
			return;
		}

		Actor actorA = contextScene->TryGetActorWithUUID(actorA_UserData->ActorUUID);
		Actor actorB = contextScene->TryGetActorWithUUID(actorB_UserData->ActorUUID);

		if (!actorA || !actorB)
		{
			return;
		}

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			ManagedMethod method = ManagedMethod::OnCollisionEnter;
			Collision collision{};

			collision.ActorID = actorB.GetUUID();
			RuntimeMethodArgument arg0(collision);
			Utils::CallMethod(actorA, method, { arg0 });

			collision.ActorID = actorA.GetUUID();
			RuntimeMethodArgument arg1(collision);
			Utils::CallMethod(actorB, method, { arg1 });
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			ManagedMethod method = ManagedMethod::OnCollisionExit;
			Collision collision{};

			if (ScriptEngine::HasValidScriptClass(actorA) && ScriptEngine::ScriptInstanceHasMethod(actorA, method))
			{
				collision.ActorID = actorB.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, actorA, { arg0 });
			}

			if (ScriptEngine::HasValidScriptClass(actorB) && ScriptEngine::ScriptInstanceHasMethod(actorB, method))
			{
				collision.ActorID = actorA.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, actorB, { arg0 });
			}
		}
	}

	void PhysicsContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (!contextScene || !contextScene->IsRunning())
		{
			return;
		}

		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			const PhysicsBodyData* triggerActorUserData = (PhysicsBodyData*)pairs[i].triggerActor->userData;
			const PhysicsBodyData* otherActorUserData = (PhysicsBodyData*)pairs[i].otherActor->userData;

			if (!triggerActorUserData)
				continue;

			if (!otherActorUserData)
			{
				// figure out how to get controller info here
				continue;
			}

			Actor triggerActor = contextScene->TryGetActorWithUUID(triggerActorUserData->ActorUUID);
			Actor otherActor = contextScene->TryGetActorWithUUID(otherActorUserData->ActorUUID);

			if (!triggerActor || !otherActor)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				ManagedMethod method = ManagedMethod::OnTriggerEnter;
				Collision collision{};

				if (ScriptEngine::HasValidScriptClass(triggerActor) && ScriptEngine::ScriptInstanceHasMethod(triggerActor, method))
				{
					collision.ActorID = otherActor.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, triggerActor, { arg0 });
				}

				if (ScriptEngine::HasValidScriptClass(otherActor) && ScriptEngine::ScriptInstanceHasMethod(otherActor, method))
				{
					collision.ActorID = triggerActor.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, otherActor, { arg0 });
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				ManagedMethod method = ManagedMethod::OnTriggerExit;
				Collision collision{};

				if (ScriptEngine::HasValidScriptClass(triggerActor) && ScriptEngine::ScriptInstanceHasMethod(triggerActor, method))
				{
					collision.ActorID = otherActor.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, triggerActor, { arg0 });
				}

				if (ScriptEngine::HasValidScriptClass(otherActor) && ScriptEngine::ScriptInstanceHasMethod(otherActor, method))
				{
					collision.ActorID = triggerActor.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, otherActor, { arg0 });
				}
			}
		}
	}

	void PhysicsContactListener::onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count)
	{
		PX_UNUSED(bodyBuffer);
		PX_UNUSED(poseBuffer);
		PX_UNUSED(count);
	}

}
