#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/3D/PhysicsTypes.h"

#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include <string>

#include <PhysX/PxPhysicsAPI.h>

namespace Vortex {

	class PhysicsUtils
	{
	public:
#define VX_PHYSICS_OVERLAP_MAX_COLLIDERS 50

		static std::array<physx::PxOverlapHit, VX_PHYSICS_OVERLAP_MAX_COLLIDERS> s_OverlapBuffer;

		static VX_FORCE_INLINE physx::PxMat44 ToPhysXMatrix(const Math::mat4& matrix)
		{
			return *(physx::PxMat44*)&matrix;
		}

		static VX_FORCE_INLINE const physx::PxVec3& ToPhysXVector(const Math::vec3& vector)
		{
			return *(physx::PxVec3*)&vector;
		}
		
		static VX_FORCE_INLINE const physx::PxVec4& ToPhysXVector(const Math::vec4& vector)
		{
			return *(physx::PxVec4*)&vector;
		}
		
		static VX_FORCE_INLINE physx::PxExtendedVec3 ToPhysXExtendedVector(const Math::vec3& vector)
		{
			return physx::PxExtendedVec3(vector.x, vector.y, vector.z);
		}
		
		static VX_FORCE_INLINE physx::PxQuat ToPhysXQuat(const Math::quaternion& quat)
		{
			return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
		}

		static VX_FORCE_INLINE physx::PxTransform ToPhysXTransform(const TransformComponent& transform)
		{
			physx::PxQuat r = ToPhysXQuat(transform.GetRotation());
			physx::PxVec3 p = ToPhysXVector(transform.Translation);
			return physx::PxTransform(p, r);
		}

		static VX_FORCE_INLINE physx::PxTransform ToPhysXTransform(const Math::mat4& transform)
		{
			Math::vec3 translation;
			Math::quaternion rotation;
			Math::vec3 scale;
			Math::DecomposeTransform(transform, translation, rotation, scale);

			physx::PxQuat r = ToPhysXQuat(rotation);
			physx::PxVec3 p = ToPhysXVector(translation);
			return physx::PxTransform(p, r);
		}

		static VX_FORCE_INLINE physx::PxTransform ToPhysXTransform(const Math::vec3& translation, const Math::quaternion& rotation)
		{
			return physx::PxTransform(ToPhysXVector(translation), ToPhysXQuat(rotation));
		}

		static VX_FORCE_INLINE Math::vec3 FromPhysXExtendedVector(const physx::PxExtendedVec3& vector)
		{
			return Math::vec3(vector.x, vector.y, vector.z);
		}

		static VX_FORCE_INLINE Math::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
		{
			return *(Math::mat4*)&matrix;
		}
		
		static VX_FORCE_INLINE Math::vec3 FromPhysXVector(const physx::PxVec3& vector)
		{
			return *(Math::vec3*)&vector;
		}
		
		static VX_FORCE_INLINE Math::vec4 FromPhysXVector(const physx::PxVec4& vector)
		{
			return *(Math::vec4*)&vector;
		}
		
		static VX_FORCE_INLINE Math::quaternion FromPhysXQuat(const physx::PxQuat& quat)
		{
			return Math::quaternion(quat.w, quat.x, quat.y, quat.z);
		}

		static VX_FORCE_INLINE Math::mat4 FromPhysXTransform(const physx::PxTransform& transform)
		{
			Math::quaternion rotation = FromPhysXQuat(transform.q);
			Math::vec3 position = FromPhysXVector(transform.p);
			return Math::Translate(position) * Math::ToMat4(rotation);
		}

		static VX_FORCE_INLINE std::string PhysXGeometryTypeToString(physx::PxGeometryType::Enum type)
		{
			switch (type)
			{
				case physx::PxGeometryType::eSPHERE:       return "Sphere";
				case physx::PxGeometryType::ePLANE:        return "Plane";
				case physx::PxGeometryType::eCAPSULE:      return "Capsule";
				case physx::PxGeometryType::eBOX:          return "Box";
				case physx::PxGeometryType::eCONVEXMESH:   return "Convex Mesh";
				case physx::PxGeometryType::eTRIANGLEMESH: return "Triangle Mesh";
				case physx::PxGeometryType::eHEIGHTFIELD:  return "Height Field";
			}

			VX_CORE_ASSERT(false, "Unknown Geometry Type!");
			return "";
		}

		static VX_FORCE_INLINE physx::PxBroadPhaseType::Enum VortexBroadphaseTypeToPhysXBroadphaseType(BroadphaseType broadphaseModel)
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

		static VX_FORCE_INLINE physx::PxFrictionType::Enum VortexFrictionTypeToPhysXFrictionType(FrictionType frictionModel)
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

		static VX_FORCE_INLINE void ReplaceInconsistentVectorAxis(Math::vec3& vector, const physx::PxVec3& replacementVector)
		{
			const uint32_t size = vector.length();

			for (uint32_t i = 0; i < size; i++)
			{
				if (vector[i] != 0.0f)
					continue;

				vector[i] = replacementVector[i];
			}
		}

		static VX_FORCE_INLINE physx::PxTransform GetLocalFrame(physx::PxRigidActor* actor)
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

		static VX_FORCE_INLINE bool OverlapGeometry(const Math::vec3& origin, const physx::PxGeometry& geometry, std::array<OverlapHit, VX_PHYSICS_OVERLAP_MAX_COLLIDERS>& buffer, uint32_t& count)
		{
			physx::PxOverlapBuffer buf(s_OverlapBuffer.data(), VX_PHYSICS_OVERLAP_MAX_COLLIDERS);
			physx::PxTransform pose = ToPhysXTransform(Math::Translate(origin));

			bool result = Physics::GetPhysicsScene()->overlap(geometry, pose, buf);

			if (result)
			{
				count = buf.nbTouches > VX_PHYSICS_OVERLAP_MAX_COLLIDERS ? VX_PHYSICS_OVERLAP_MAX_COLLIDERS : buf.nbTouches;

				for (uint32_t i = 0; i < count; i++)
				{
					PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)s_OverlapBuffer[i].actor->userData;
					buffer[i].ActorID = physicsBodyData->ActorUUID;
				}
			}

			return result;
		}

	};

}