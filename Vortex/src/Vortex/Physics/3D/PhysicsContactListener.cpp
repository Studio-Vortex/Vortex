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

		PhysicsBodyData* entityDataA = (PhysicsBodyData*)pairHeader.actors[0]->userData;
		PhysicsBodyData* entityDataB = (PhysicsBodyData*)pairHeader.actors[1]->userData;

		if (!entityDataA || !entityDataB)
			return;

		Entity entityA = contextScene->TryGetEntityWithUUID(entityDataA->EntityUUID);
		Entity entityB = contextScene->TryGetEntityWithUUID(entityDataB->EntityUUID);

		if (!entityA || !entityB)
			return;

		if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
		{
			if (entityA.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityA, entityB, collision);
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityB, entityA, collision);
			}
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			if (entityA.HasComponent<ScriptComponent>())
			{
				ScriptEngine::OnCollisionEndEntity(entityA, entityB);
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				ScriptEngine::OnCollisionEndEntity(entityB, entityA);
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

			PhysicsBodyData* triggerActorPhysicsBodyData = (PhysicsBodyData*)pairs[i].triggerActor->userData;
			PhysicsBodyData* otherActorPhysicsBodyData = (PhysicsBodyData*)pairs[i].otherActor->userData;

			if (!triggerActorPhysicsBodyData || !otherActorPhysicsBodyData)
				continue;

			Entity triggerEntity = contextScene->TryGetEntityWithUUID(triggerActorPhysicsBodyData->EntityUUID);
			Entity otherEntity = contextScene->TryGetEntityWithUUID(otherActorPhysicsBodyData->EntityUUID);

			if (!triggerEntity || !otherEntity)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_CCD)
			{
				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					ScriptEngine::OnTriggerBeginEntity(triggerEntity, otherEntity);
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					ScriptEngine::OnTriggerBeginEntity(otherEntity, triggerEntity);
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (triggerEntity.HasComponent<ScriptComponent>())
				{
					ScriptEngine::OnTriggerEndEntity(triggerEntity, otherEntity);
				}

				if (otherEntity.HasComponent<ScriptComponent>())
				{
					ScriptEngine::OnTriggerEndEntity(otherEntity, triggerEntity);
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
