#include "vxpch.h"
#include "PhysicsContactListener.h"

#include "Vortex/Physics/3D/Physics.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

namespace Vortex {

	namespace Utils {

		bool HasValidScriptInstance(Actor entity)
		{
			if (!entity.HasComponent<ScriptComponent>())
			{
				return false;
			}

			const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();
			return ScriptEngine::ActorClassExists(scriptComponent.ClassName);
		}

		bool ScriptInstanceHasMethod(Actor entity, ManagedMethod method)
		{
			SharedReference<ScriptInstance> instance = ScriptEngine::GetActorScriptInstance(entity.GetUUID());
			if (instance == nullptr)
			{
				return false;
			}

			return instance->MethodExists(method);
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

			Actor entity = contextScene->TryGetActorWithUUID(jointData->ActorUUID);

			if (!entity)
			{
				continue;
			}

			Actor connectedEntity;

			if (entity.HasComponent<FixedJointComponent>())
			{
				const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
				UUID connectedEntityUUID = fixedJointComponent.ConnectedActor;

				if (Actor attachedEntity = contextScene->TryGetActorWithUUID(connectedEntityUUID))
				{
					connectedEntity = attachedEntity;
				}
			}

			if (!connectedEntity)
			{
				continue;
			}

			if (nativeJoint->is<physx::PxFixedJoint>())
			{
				physx::PxFixedJoint* fixedJoint = nativeJoint->is<physx::PxFixedJoint>();
				const std::pair<Math::vec3, Math::vec3>& forceAndTorque = Physics::GetLastReportedFixedJointForces(fixedJoint);

				RuntimeMethodArgument arg0(forceAndTorque);

				if (Utils::HasValidScriptInstance(entity))
				{
					ScriptEngine::Invoke(ManagedMethod::OnFixedJointDisconnected, entity, { arg0 });
				}

				if (Utils::HasValidScriptInstance(connectedEntity))
				{
					ScriptEngine::Invoke(ManagedMethod::OnFixedJointDisconnected, connectedEntity, { arg0 });
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

		const PhysicsBodyData* entityA_UserData = (PhysicsBodyData*)pairHeader.actors[0]->userData;
		const PhysicsBodyData* entityB_UserData = (PhysicsBodyData*)pairHeader.actors[1]->userData;

		if (!entityA_UserData || !entityB_UserData)
		{
			return;
		}

		Actor entityA = contextScene->TryGetActorWithUUID(entityA_UserData->ActorUUID);
		Actor entityB = contextScene->TryGetActorWithUUID(entityB_UserData->ActorUUID);

		if (!entityA || !entityB)
		{
			return;
		}

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			ManagedMethod method = ManagedMethod::OnCollisionEnter;
			Collision collision{};

			if (Utils::HasValidScriptInstance(entityA) && Utils::ScriptInstanceHasMethod(entityA, method))
			{
				collision.EntityID = entityB.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, entityA, { arg0 });
			}

			if (Utils::HasValidScriptInstance(entityB) && Utils::ScriptInstanceHasMethod(entityB, method))
			{
				collision.EntityID = entityA.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, entityB, { arg0 });
			}
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			ManagedMethod method = ManagedMethod::OnCollisionExit;
			Collision collision{};

			if (Utils::HasValidScriptInstance(entityA) && Utils::ScriptInstanceHasMethod(entityA, method))
			{
				collision.EntityID = entityB.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, entityA, { arg0 });
			}

			if (Utils::HasValidScriptInstance(entityB) && Utils::ScriptInstanceHasMethod(entityB, method))
			{
				collision.EntityID = entityA.GetUUID();
				RuntimeMethodArgument arg0(collision);
				ScriptEngine::Invoke(method, entityB, { arg0 });
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

			Actor triggerEntity = contextScene->TryGetActorWithUUID(triggerActorUserData->ActorUUID);
			Actor otherEntity = contextScene->TryGetActorWithUUID(otherActorUserData->ActorUUID);

			if (!triggerEntity || !otherEntity)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				ManagedMethod method = ManagedMethod::OnTriggerEnter;
				Collision collision{};

				if (Utils::HasValidScriptInstance(triggerEntity) && Utils::ScriptInstanceHasMethod(triggerEntity, method))
				{
					collision.EntityID = otherEntity.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, triggerEntity, { arg0 });
				}

				if (Utils::HasValidScriptInstance(otherEntity) && Utils::ScriptInstanceHasMethod(otherEntity, method))
				{
					collision.EntityID = triggerEntity.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, otherEntity, { arg0 });
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				ManagedMethod method = ManagedMethod::OnTriggerExit;
				Collision collision{};

				if (Utils::HasValidScriptInstance(triggerEntity) && Utils::ScriptInstanceHasMethod(triggerEntity, method))
				{
					collision.EntityID = otherEntity.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, triggerEntity, { arg0 });
				}

				if (Utils::HasValidScriptInstance(otherEntity) && Utils::ScriptInstanceHasMethod(otherEntity, method))
				{
					collision.EntityID = triggerEntity.GetUUID();
					RuntimeMethodArgument arg0(collision);
					ScriptEngine::Invoke(method, otherEntity, { arg0 });
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
