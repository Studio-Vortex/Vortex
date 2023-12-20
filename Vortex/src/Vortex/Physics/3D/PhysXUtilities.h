#pragma once

#include "Vortex/Math/Math.h"

#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	namespace Utils {

		static std::array<physx::PxOverlapHit, OVERLAP_MAX_COLLIDERS> s_OverlapBuffer;

		static physx::PxBroadPhaseType::Enum VortexBroadphaseTypeToPhysXBroadphaseType(BroadphaseType broadphaseModel)
		{
			switch (broadphaseModel)
			{
				case BroadphaseType::SweepAndPrune:     return physx::PxBroadPhaseType::eSAP;
				case BroadphaseType::MultiBoxPrune:     return physx::PxBroadPhaseType::eMBP;
				case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
			}

			VX_CORE_ASSERT(false, "Unknown Broadphase Type!");
			return physx::PxBroadPhaseType::eABP;
		}

		static physx::PxFrictionType::Enum VortexFrictionTypeToPhysXFrictionType(FrictionType frictionModel)
		{
			switch (frictionModel)
			{
				case Vortex::FrictionType::OneDirectional: return physx::PxFrictionType::eONE_DIRECTIONAL;
				case Vortex::FrictionType::Patch:          return physx::PxFrictionType::ePATCH;
				case Vortex::FrictionType::TwoDirectional: return physx::PxFrictionType::eTWO_DIRECTIONAL;
			}

			VX_CORE_ASSERT(false, "Unknown Friction Type!");
			return physx::PxFrictionType::ePATCH;
		}

		static void ReplaceInconsistentVectorAxis(Math::vec3& vector, const physx::PxVec3& replacementVector)
		{
			const uint32_t size = vector.length();

			for (uint32_t i = 0; i < size; i++)
			{
				if (vector[i] != 0.0f)
					continue;

				vector[i] = replacementVector[i];
			}
		}

		static physx::PxTransform GetLocalFrame(physx::PxRigidActor* actor)
		{
			PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)actor->userData;
			Scene* contextScene = physicsBodyData->ContextScene;
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
			Actor entity = contextScene->TryGetActorWithUUID(physicsBodyData->ActorUUID);
			VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

			const TransformComponent& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::quaternion rotation = worldSpaceTransform.GetRotation();
			Math::vec3 globalNormal = rotation * Math::vec3(0.0f, 0.0f, -1.0f);
			Math::vec3 globalAxis = rotation * Math::vec3(0.0f, 1.0f, 0.0f);

			physx::PxVec3 localAnchor = actor->getGlobalPose().transformInv(ToPhysXVector(worldSpaceTransform.Translation));
			physx::PxVec3 localNormal = actor->getGlobalPose().rotateInv(ToPhysXVector(globalNormal));
			physx::PxVec3 localAxis = actor->getGlobalPose().rotateInv(ToPhysXVector(globalAxis));

			physx::PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));

			physx::PxTransform localFrame;
			localFrame.p = localAnchor;
			localFrame.q = physx::PxQuat(rot);
			localFrame.q.normalize();

			return localFrame;
		}

		static bool OverlapGeometry(const Math::vec3& origin, const physx::PxGeometry& geometry, std::array<OverlapHit, OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
		{
			physx::PxOverlapBuffer buf(s_OverlapBuffer.data(), OVERLAP_MAX_COLLIDERS);
			physx::PxTransform pose = ToPhysXTransform(Math::Translate(origin));

			bool result = Physics::GetPhysicsScene()->overlap(geometry, pose, buf);

			if (result)
			{
				count = buf.nbTouches > OVERLAP_MAX_COLLIDERS ? OVERLAP_MAX_COLLIDERS : buf.nbTouches;

				for (uint32_t i = 0; i < count; i++)
				{
					PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)s_OverlapBuffer[i].actor->userData;
					buffer[i].ActorID = physicsBodyData->ActorUUID;
				}
			}

			return result;
		}

	}

}