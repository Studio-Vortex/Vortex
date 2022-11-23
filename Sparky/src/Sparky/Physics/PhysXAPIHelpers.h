#include "Sparky/Core/Math.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Sparky {

	static std::tuple<Math::vec3, Math::quaternion, Math::vec3> GetTransformDecomposition(const Math::mat4& transform)
	{
		Math::vec3 scale, translation, skew;
		Math::vec4 perspective;
		Math::quaternion orientation;
		Math::Decompose(transform, scale, orientation, translation, skew, perspective);

		return { translation, orientation, scale };
	}

	static physx::PxQuat ToPhysXQuat(const Math::quaternion& quat)
	{
		return physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
	}

	static physx::PxVec3 ToPhysXVector(const Math::vec3& vector)
	{
		return physx::PxVec3(vector.x, vector.y, vector.z);
	}

	static physx::PxVec4 ToPhysXVector(const Math::vec4& vector)
	{
		return physx::PxVec4(vector.x, vector.y, vector.z, vector.w);
	}

	static physx::PxExtendedVec3 ToPhysxExtendedVector(const Math::vec3& vector)
	{
		return physx::PxExtendedVec3(vector.x, vector.y, vector.z);
	}

	static physx::PxTransform ToPhysXTransform(const Math::mat4& matrix)
	{
		physx::PxQuat r = ToPhysXQuat(Math::Normalize(Math::ToQuaternion(matrix)));
		physx::PxVec3 p = ToPhysXVector(Math::vec3(matrix[3]));
		return physx::PxTransform(p, r);
	}

	static physx::PxMat44 ToPhysXMatrix(const Math::mat4& matrix)
	{
		return *(physx::PxMat44*)&matrix;
	}

	static Math::quaternion FromPhysXQuat(const physx::PxQuat& quat)
	{
		return Math::quaternion(quat.w, quat.x, quat.y, quat.z);
	}

	static Math::vec3 FromPhysXExtendedVector(const physx::PxExtendedVec3& vector)
	{
		return Math::vec3(vector.x, vector.y, vector.z);
	}

	static Math::vec3 FromPhysXVector(const physx::PxVec3& vector)
	{
		return Math::vec3(vector.x, vector.y, vector.z);
	}

	static Math::vec4 FromPhysXVector(const physx::PxVec4& vector)
	{
		return Math::vec4(vector.x, vector.y, vector.z, vector.w);
	}

	static Math::mat4 FromPhysXTransform(const physx::PxTransform& transform)
	{
		Math::quaternion rotation = FromPhysXQuat(transform.q);
		Math::vec3 position = FromPhysXVector(transform.p);
		return Math::Translate(position) * Math::ToMat4(rotation);
	}

	static Math::mat4 FromPhysXMatrix(const physx::PxMat44& matrix)
	{
		return *(Math::mat4*)&matrix;
	}

}
