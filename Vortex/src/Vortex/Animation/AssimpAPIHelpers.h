#pragma once

#include "Vortex/Math/Math.h"

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
		Math::mat4 to = Math::Identity();

		//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
		to[0][0] = matrix.a1; to[1][0] = matrix.a2; to[2][0] = matrix.a3; to[3][0] = matrix.a4;
		to[0][1] = matrix.b1; to[1][1] = matrix.b2; to[2][1] = matrix.b3; to[3][1] = matrix.b4;
		to[0][2] = matrix.c1; to[1][2] = matrix.c2; to[2][2] = matrix.c3; to[3][2] = matrix.c4;
		to[0][3] = matrix.d1; to[1][3] = matrix.d2; to[2][3] = matrix.d3; to[3][3] = matrix.d4;
		return to;
	}

	static inline Math::quaternion FromAssimpQuat(const aiQuaternion& quat)
	{
		return Math::quaternion{ quat.w, quat.x, quat.y, quat.z };
	}

}
