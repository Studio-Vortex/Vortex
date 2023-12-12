#pragma once

#include "Vortex/Math/Math.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Physics/3D/PhysXTypes.h"

#include <PhysX/PxPhysicsAPI.h>

#include <string>

namespace Vortex {

	static inline physx::PxMat44 ToPhysXMatrix(const Math::mat4& matrix) { return *(physx::PxMat44*)&matrix; }
	static inline const physx::PxVec3& ToPhysXVector(const Math::vec3& vector) { return *(physx::PxVec3*)&vector; }
	static inline const physx::PxVec4& ToPhysXVector(const Math::vec4& vector) { return *(physx::PxVec4*)&vector; }
	static inline physx::PxExtendedVec3 ToPhysXExtendedVector(const Math::vec3& vector) { return physx::PxExtendedVec3(vector.x, vector.y, vector.z); }
	static inline physx::PxQuat ToPhysXQuat(const Math::quaternion& quat) { return physx::PxQuat(quat.x, quat.y, quat.z, quat.w); }

	static inline physx::PxTransform ToPhysXTransform(const TransformComponent& transform)
	{
		physx::PxQuat r = ToPhysXQuat(transform.GetRotation());
		physx::PxVec3 p = ToPhysXVector(transform.Translation);
		return physx::PxTransform(p, r);
	}

	static inline physx::PxTransform ToPhysXTransform(const Math::mat4& transform)
	{
		Math::vec3 translation;
		Math::quaternion rotation;
		Math::vec3 scale;
		Math::DecomposeTransform(transform, translation, rotation, scale);

		physx::PxQuat r = ToPhysXQuat(rotation);
		physx::PxVec3 p = ToPhysXVector(translation);
		return physx::PxTransform(p, r);
	}

	static inline physx::PxTransform ToPhysXTransform(const Math::vec3& translation, const Math::quaternion& rotation)
	{
		return physx::PxTransform(ToPhysXVector(translation), ToPhysXQuat(rotation));
	}

	static inline Math::vec3 FromPhysXExtendedVector(const physx::PxExtendedVec3& vector)
	{
		return Math::vec3(vector.x, vector.y, vector.z);
	}

	static inline Math::mat4 FromPhysXMatrix(const physx::PxMat44& matrix) { return *(Math::mat4*)&matrix; }
	static inline Math::vec3 FromPhysXVector(const physx::PxVec3& vector) { return *(Math::vec3*)&vector; }
	static inline Math::vec4 FromPhysXVector(const physx::PxVec4& vector) { return *(Math::vec4*)&vector; }
	static inline Math::quaternion FromPhysXQuat(const physx::PxQuat& quat) { return Math::quaternion(quat.w, quat.x, quat.y, quat.z); }

	static inline Math::mat4 FromPhysXTransform(const physx::PxTransform& transform)
	{
		Math::quaternion rotation = FromPhysXQuat(transform.q);
		Math::vec3 position = FromPhysXVector(transform.p);
		return Math::Translate(position) * Math::ToMat4(rotation);
	}

	static inline std::string PhysXGeometryTypeToString(physx::PxGeometryType::Enum type)
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

}
