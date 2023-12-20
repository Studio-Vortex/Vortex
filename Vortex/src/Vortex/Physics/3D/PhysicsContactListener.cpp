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
				ManagedMethod method = ManagedMethod::OnFixedJointDisconnected;
				physx::PxFixedJoint* fixedJoint = nativeJoint->is<physx::PxFixedJoint>();
				const std::pair<Math::vec3, Math::vec3>& forceAndTorque = Physics::GetLastReportedFixedJointForces(fixedJoint);

				RuntimeMethodArgument arg0(forceAndTorque);

				Utils::CallMethod(actor, method, { arg0 });
				Utils::CallMethod(connectedActor, method, { arg0 });
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

		const PhysicsBodyData* actorA_UserData = (const PhysicsBodyData*)pairHeader.actors[0]->userData;
		const PhysicsBodyData* actorB_UserData = (const PhysicsBodyData*)pairHeader.actors[1]->userData;

		if (!actorA_UserData)
		{
			return;
		}

		if (!actorB_UserData)
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

			collision.ActorID = actorB.GetUUID();
			RuntimeMethodArgument arg0(collision);
			Utils::CallMethod(actorA, method, { arg0 });

			collision.ActorID = actorA.GetUUID();
			RuntimeMethodArgument arg1(collision);
			Utils::CallMethod(actorB, method, { arg1 });
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

			const PhysicsBodyData* triggerActorUserData = (const PhysicsBodyData*)pairs[i].triggerActor->userData;
			PhysicsBodyData* otherActorUserData = (PhysicsBodyData*)pairs[i].otherActor->userData;

			if (!triggerActorUserData)
				continue;

			bool foundController = false;

			// TODO: currently we just look for the first controller
			// in the scene and use is as the 'otherActor', this will probably need to be
			// expanded in the future to find the actual controller, (i.e. if we had a scene with multiple character controllers we would have some weird bugs with triggers)
			if (!otherActorUserData)
			{
				// since all rigid actors have userData because Physics creates it for them
				// lets try to look for a controller, because nothing else in the scene could be the 'otherActor'
				const std::unordered_map<UUID, physx::PxController*>& controllers = Physics::GetControllers();

				for (const auto& [actorUUID, pxController] : controllers)
				{
					otherActorUserData = new PhysicsBodyData();
					otherActorUserData->ActorUUID = actorUUID;
					otherActorUserData->ContextScene = contextScene;
					foundController = true;
					break;
				}

				// we didn't find a controller
				if (otherActorUserData == nullptr)
					continue;
			}

			Actor triggerActor = contextScene->TryGetActorWithUUID(triggerActorUserData->ActorUUID);
			Actor otherActor = contextScene->TryGetActorWithUUID(otherActorUserData->ActorUUID);

#ifdef VX_DEBUG
			const std::string& triggerActorName = triggerActor.GetName();
			const std::string& otherActorName = otherActor.GetName();
#endif

			if (foundController)
			{
				delete otherActorUserData;
			}

			if (!triggerActor || !otherActor)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				ManagedMethod method = ManagedMethod::OnTriggerEnter;
				Collision collision{};

				collision.ActorID = otherActor.GetUUID();
				RuntimeMethodArgument arg0(collision);
				Utils::CallMethod(triggerActor, method, { arg0 });

				collision.ActorID = triggerActor.GetUUID();
				RuntimeMethodArgument arg1(collision);
				Utils::CallMethod(otherActor, method, { arg1 });
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				ManagedMethod method = ManagedMethod::OnTriggerExit;
				Collision collision{};

				collision.ActorID = otherActor.GetUUID();
				RuntimeMethodArgument arg0(collision);
				Utils::CallMethod(triggerActor, method, { arg0 });

				collision.ActorID = triggerActor.GetUUID();
				RuntimeMethodArgument arg1(collision);
				Utils::CallMethod(otherActor, method, { arg1 });
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
