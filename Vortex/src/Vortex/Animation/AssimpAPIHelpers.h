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
		ret[0][0] = matrix.a1; ret[1][0] = matrix.a2; ret[2][0] = matrix.a3; ret[3][0] = matrix.a4;
		ret[0][1] = matrix.b1; ret[1][1] = matrix.b2; ret[2][1] = matrix.b3; ret[3][1] = matrix.b4;
		ret[0][2] = matrix.c1; ret[1][2] = matrix.c2; ret[2][2] = matrix.c3; ret[3][2] = matrix.c4;
		ret[0][3] = matrix.d1; ret[1][3] = matrix.d2; ret[2][3] = matrix.d3; ret[3][3] = matrix.d4;
		return ret;
	}

	static inline Math::quaternion FromAssimpQuat(const aiQuaternion& quat)
	{
		return Math::quaternion{ quat.w, quat.x, quat.y, quat.z };
	}

}
