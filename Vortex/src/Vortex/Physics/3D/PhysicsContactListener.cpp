#include "vxpch.h"
#include "PhysicsContactListener.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	namespace Utils {

		bool HasValidScriptInstance(Entity entity)
		{
			if (!entity.HasComponent<ScriptComponent>())
			{
				return false;
			}

			const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();
			return ScriptEngine::EntityClassExists(scriptComponent.ClassName);
		}

	}

	void PhysicsContactListener::onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (!contextScene || !contextScene->IsRunning())
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxJoint* nativeJoint = (physx::PxJoint*)constraints[i].externalReference;
			ConstrainedJointData* jointData = (ConstrainedJointData*)nativeJoint->userData;

			nativeJoint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);

			Entity entity = contextScene->TryGetEntityWithUUID(jointData->EntityUUID);

			if (!entity)
			{
				continue;
			}

			Entity connectedEntity;

			if (entity.HasComponent<FixedJointComponent>())
			{
				const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
				UUID connectedEntityUUID = fixedJointComponent.ConnectedEntity;

				if (Entity attachedEntity = contextScene->TryGetEntityWithUUID(connectedEntityUUID))
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

				if (Utils::HasValidScriptInstance(entity))
				{
					ScriptEngine::OnFixedJointDisconnected(entity, forceAndTorque);
				}

				if (Utils::HasValidScriptInstance(connectedEntity))
				{
					ScriptEngine::OnFixedJointDisconnected(connectedEntity, forceAndTorque);
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
				ScriptEngine::OnCollisionEnterEntity(entityA, collision);
			}

			if (entityB.HasComponent<ScriptComponent>())
			{
				Collision collision{};
				collision.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionEnterEntity(entityB, collision);
			}
		}
		else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
		{
			if (Utils::HasValidScriptInstance(entityA))
			{
				Collision collision{};
				collision.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionExitEntity(entityA, collision);
			}

			if (Utils::HasValidScriptInstance(entityB))
			{
				Collision collision{};
				collision.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionExitEntity(entityB, collision);
			}
		}
	}

	void PhysicsContactListener::onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
	{
		Scene* contextScene = ScriptEngine::GetContextScene();

		if (!contextScene || !contextScene->IsRunning())
			return;

		for (uint32_t i = 0; i < count; i++)
		{
			if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				continue;

			PhysicsBodyData* triggerActorUserData = (PhysicsBodyData*)pairs[i].triggerActor->userData;
			PhysicsBodyData* otherActorUserData = (PhysicsBodyData*)pairs[i].otherActor->userData;

			if (!triggerActorUserData)
				continue;

			if (!otherActorUserData)
			{
				// figure out how to get controller info here
				continue;
			}

			Entity triggerEntity = contextScene->TryGetEntityWithUUID(triggerActorUserData->EntityUUID);
			Entity otherEntity = contextScene->TryGetEntityWithUUID(otherActorUserData->EntityUUID);

			if (!triggerEntity || !otherEntity)
				continue;

			if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND)
			{
				if (Utils::HasValidScriptInstance(triggerEntity))
				{
					Collision collision{};
					collision.EntityID = otherEntity.GetUUID();
					ScriptEngine::OnTriggerEnterEntity(triggerEntity, collision);
				}

				if (Utils::HasValidScriptInstance(otherEntity))
				{
					Collision collision{};
					collision.EntityID = triggerEntity.GetUUID();
					ScriptEngine::OnTriggerEnterEntity(otherEntity, collision);
				}
			}
			else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
			{
				if (Utils::HasValidScriptInstance(triggerEntity))
				{
					Collision collision{};
					collision.EntityID = otherEntity.GetUUID();
					ScriptEngine::OnTriggerExitEntity(triggerEntity, collision);
				}

				if (Utils::HasValidScriptInstance(otherEntity))
				{
					Collision collision{};
					collision.EntityID = triggerEntity.GetUUID();
					ScriptEngine::OnTriggerExitEntity(otherEntity, collision);
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
