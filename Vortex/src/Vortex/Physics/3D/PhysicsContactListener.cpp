#include "vxpch.h"
#include "PhysicsContactListener.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	void PhysicsContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		PX_UNUSED(constraints);
		PX_UNUSED(count);
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

		if (contextScene == nullptr || !contextScene->IsRunning())
			return;

		bool removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
		bool removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

		PhysicsBodyData* entityA_UserData = (PhysicsBodyData*)pairHeader.actors[0]->userData;
		PhysicsBodyData* entityB_UserData = (PhysicsBodyData*)pairHeader.actors[1]->userData;

		if (!entityA_UserData || !entityB_UserData)
			return;

		Entity entityA = contextScene->TryGetEntityWithUUID(entityA_UserData->EntityUUID);
		Entity entityB = contextScene->TryGetEntityWithUUID(entityB_UserData->EntityUUID);

		if (!entityA || !entityB)
			return;

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			if (entityA.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityA, collision);
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityB, collision);
			}
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			if (entityA.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionEndEntity(entityA, collision);
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionEndEntity(entityB, collision);
			}
		}
	}

	void PhysicsContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (contextScene == nullptr || !contextScene->IsRunning())
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			PhysicsBodyData* triggerActorUserData = (PhysicsBodyData*)pairs[i].triggerActor->userData;
			PhysicsBodyData* otherActorUserData = (PhysicsBodyData*)pairs[i].otherActor->userData;

			if (!triggerActorUserData || !otherActorUserData)
				continue;

			Entity triggerEntity = contextScene->TryGetEntityWithUUID(triggerActorUserData->EntityUUID);
			Entity otherEntity = contextScene->TryGetEntityWithUUID(otherActorUserData->EntityUUID);

			if (!triggerEntity || !otherEntity)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_CCD)
			{
				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					Collision collision{};
					collision.EntityID = otherEntity.GetUUID();
					ScriptEngine::OnTriggerBeginEntity(triggerEntity, collision);
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					Collision collision{};
					collision.EntityID = triggerEntity.GetUUID();
					ScriptEngine::OnTriggerBeginEntity(otherEntity, collision);
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					Collision collision{};
					collision.EntityID = otherEntity.GetUUID();
					ScriptEngine::OnTriggerEndEntity(triggerEntity, collision);
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					Collision collision{};
					collision.EntityID = triggerEntity.GetUUID();
					ScriptEngine::OnTriggerEndEntity(otherEntity, collision);
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
