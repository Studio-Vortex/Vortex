#pragma once

#include "Vortex/Core/Math.h"

#include <assimp/quaternion.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>

namespace Vortex {

	static inline Math::vec3 FromAssimpVec3(const aiVector3D& vector)
	{
		Math::vec3 ret{ vector.x, vector.y, vector.z };
		return ret;
	}

	static inline Math::mat4 FromAssimpMat4(const aiMatrix4x4& matrix)
	{
		Math::mat4 ret = Math::Identity();

		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		ret[0] = Math::vec4(matrix.a1, matrix.b1, matrix.c1, matrix.d1);
		ret[1] = Math::vec4(matrix.a2, matrix.b2, matrix.c2, matrix.d2);
		ret[2] = Math::vec4(matrix.a3, matrix.b3, matrix.c3, matrix.d3);
		ret[3] = Math::vec4(matrix.a4, matrix.b4, matrix.c4, matrix.d4);
		return ret;
	}

	static inline Math::quaternion FromAssimpQuat(const aiQuaternion& quat)
	{
		return Math::quaternion{ quat.w, quat.x, quat.y, quat.z };
	}

}
