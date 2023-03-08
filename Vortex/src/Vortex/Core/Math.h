#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Log.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Vortex::Math {

	typedef glm::vec2 vec2;
	typedef glm::vec3 vec3;
	typedef glm::vec4 vec4;
	typedef glm::ivec2 ivec2;
	typedef glm::ivec3 ivec3;
	typedef glm::ivec4 ivec4;
	typedef glm::mat2 mat2;
	typedef glm::mat3 mat3;
	typedef glm::mat4 mat4;
	typedef glm::quat quaternion;

	static constexpr float PI = glm::pi<float>();
	static constexpr float TWO_PI = glm::two_pi<float>();
	static constexpr double PI_D = glm::pi<double>();

	struct AABB
	{
		vec3 Min;
		vec3 Max;

		AABB()
			: Min(0.0f), Max(0.0f) { }

		AABB(const vec3& min, const vec3& max)
			: Min(min), Max(max) { }
	};

	template <typename T>
	static auto Deg2Rad(T degrees)
	{
		return glm::radians(degrees);
	}

	template <typename T>
	static auto Rad2Deg(T radians)
	{
		return glm::degrees(radians);
	}

	template <typename T>
	static auto Perspective(T verticalFOV, T aspectRatio, T zNear, T zFar)
	{
		return glm::perspective(verticalFOV, aspectRatio, zNear, zFar);
	}

	template <typename T>
	static auto PerspectiveFOV(T FOV, T width, T height, T zNear, T zFar)
	{
		return glm::perspectiveFov(FOV, width, height, zNear, zFar);
	}

	static auto LookAt(const Math::vec3& eyePos, const Math::vec3& point, const Math::vec3& up)
	{
		return glm::lookAt(eyePos, point, up);
	}

	template <typename T>
	static auto Ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		return glm::ortho<T>(left, right, bottom, top, zNear, zFar);
	}

	template <typename T, typename Z>
	static auto Lerp(T v, T v1, Z z)
	{
		return glm::lerp(v, v1, z);
	}

	template <typename T, typename Z>
	static auto Slerp(T v, T v1, Z z)
	{
		return glm::slerp(v, v1, z);
	}

	static auto Translate(const vec3& translation)
	{
		return glm::translate(mat4(1.0f), translation);
	}

	static auto Rotate(quaternion v, const vec3& axis)
	{
		return glm::rotate(v, axis);
	}

	static auto Rotate(float rotation, const vec3& axis)
	{
		return glm::rotate(mat4(1.0f), rotation, axis);
	}

	static auto Scale(const vec3& scale)
	{
		return glm::scale(mat4(1.0f), scale);
	}

	template <typename T>
	static auto Round(T value)
	{
		return glm::round(value);
	}

	template <typename T>
	static auto Max(T x, T y)
	{
		return glm::max(x, y);
	}

	template <typename T>
	static auto Min(T x, T y)
	{
		return glm::min(x, y);
	}

	template <typename T>
	static auto Clamp(T v, T min, T max)
	{
		return glm::clamp(v, min, max);
	}

	template <typename T>
	static auto Log(T v)
	{
		return glm::log(v);
	}

	template <typename T>
	static auto Abs(T v)
	{
		return glm::abs(v);
	}

	template <typename T>
	static auto Sqrt(T x)
	{
		return glm::sqrt(x);
	}

	template <typename T>
	static auto Cos(T v)
	{
		return glm::cos(v);
	}

	template <typename T>
	static auto Acos(T v)
	{
		return glm::acos(v);
	}

	template <typename T>
	static auto Sin(T v)
	{
		return glm::sin(v);
	}

	template <typename T>
	static auto Tan(T v)
	{
		return glm::tan(v);
	}

	template <typename T>
	static auto Atan(T v)
	{
		return glm::atan(v);
	}

	template <typename T>
	static auto Atan2(T v0, T v1)
	{
		return glm::atan2(v0, v1);
	}

	template <typename T>
	static auto Normalize(T v)
	{
		return glm::normalize(v);
	}

	template <typename T>
	static auto ToMat4(T v)
	{
		return glm::toMat4(v);
	}
	
	template <typename T>
	static auto Cross(T v1, T v2)
	{
		return glm::cross(v1, v2);
	}

	template <typename T>
	static auto Dot(T v1, T v2)
	{
		return glm::dot(v1, v2);
	}

	template <typename T>
	static auto Distance(T v0, T v1)
	{
		return glm::distance(v0, v1);
	}

	template <typename T, typename K>
	static auto Mix(const T& v0, const T& v1, K c0)
	{
		return glm::mix(v0, v1, c0);
	}

	template <typename T>
	static auto Quaternion(T v)
	{
		return glm::quat(v);
	}

	template <typename T>
	static auto ToQuaternion(T v)
	{
		return glm::toQuat(v);
	}

	template <typename T, typename U>
	static auto AngleAxis(T angle, const U& axis)
	{
		return glm::angleAxis(angle, axis);
	}

	static quaternion GetOrientation(float pitch, float yaw, float roll)
	{
		return quaternion(vec3(pitch, yaw, roll));
	}

	static quaternion GetOrientation(const vec3& v)
	{
		return quaternion(v);
	}

	static auto Transpose(const mat4& matrix)
	{
		return glm::transpose(matrix);
	}

	template <typename T>
	static auto Inverse(const T& v)
	{
		return glm::inverse(v);
	}

	static auto Identity()
	{
		return mat4(1.0f);
	}

	template <typename T>
	static auto ValuePtr(const T& value)
	{
		return glm::value_ptr(value);
	}
	
	template <typename T>
	static auto ValuePtr(T& value)
	{
		return glm::value_ptr(value);
	}

	static auto EulerAngles(const quaternion& rotationQuat)
	{
		return glm::eulerAngles(rotationQuat);
	}

	static auto Decompose(const mat4& transform, vec3& scale, quaternion& orientation, vec3& translation, vec3& skew, vec4& perspective)
	{
		return glm::decompose(transform, scale, orientation, translation, skew, perspective);
	}

	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale)
	{
		using namespace glm;
		using T = float;

		mat4 LocalMatrix(transform);

		// Normalize the matrix.
		if (epsilonEqual(LocalMatrix[3][3], static_cast<float>(0), epsilon<T>()))
			return false;

		// First, isolate perspective.  This is the messiest.
		if (
			epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
			epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		{
			// Clear the perspective partition
			LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
			LocalMatrix[3][3] = static_cast<T>(1);
		}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3]{}, Pdum3{};

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = glm::detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = glm::detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = glm::detail::scale(Row[2], static_cast<T>(1));

		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
#if 0
		Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		if (dot(Row[0], Pdum3) < 0)
		{
			for (length_t i = 0; i < 3; i++)
			{
				scale[i] *= static_cast<T>(-1);
				Row[i] *= static_cast<T>(-1);
			}
		}
#endif

		rotation.y = asin(-Row[0][2]);
		if (cos(rotation.y) != 0) {
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else {
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}


		return true;
	}

	static bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::quat& rotation, glm::vec3& scale)
	{
		using namespace glm;
		using T = float;

		auto ScaleFunc = [](const glm::vec3& v, float desiredLength) -> glm::vec3
		{
			return v * desiredLength / length(v);
		};

		mat4 LocalMatrix(transform);

		if (epsilonEqual(LocalMatrix[3][3], static_cast<T>(0), epsilon<T>()))
			return false;

		// Assume matrix is already normalized
		//VX_CORE_ASSERT(epsilonEqual(LocalMatrix[3][3], static_cast<T>(1), static_cast<T>(0.00001)), "");
		//for (length_t i = 0; i < 4; ++i)
		//	for (length_t j = 0; j < 4; ++j)
		//		LocalMatrix[i][j] /= LocalMatrix[3][3];

		// Ignore perspective
		//VX_CORE_ASSERT(
			//epsilonEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) &&
			//epsilonEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) &&
			//epsilonEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()), ""
		//);
		//// perspectiveMatrix is used to solve for perspective, but it also provides
		//// an easy way to test for singularity of the upper 3x3 component.
		//mat<4, 4, T, Q> PerspectiveMatrix(LocalMatrix);
		//
		//for (length_t i = 0; i < 3; i++)
		//	PerspectiveMatrix[i][3] = static_cast<T>(0);
		//PerspectiveMatrix[3][3] = static_cast<T>(1);
		//
		///// TODO: Fixme!
		//if (epsilonEqual(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
		//	return false;
		//
		//// First, isolate perspective.  This is the messiest.
		//if (
		//	epsilonNotEqual(LocalMatrix[0][3], static_cast<T>(0), epsilon<T>()) ||
		//	epsilonNotEqual(LocalMatrix[1][3], static_cast<T>(0), epsilon<T>()) ||
		//	epsilonNotEqual(LocalMatrix[2][3], static_cast<T>(0), epsilon<T>()))
		//{
		//	// rightHandSide is the right hand side of the equation.
		//	vec<4, T, Q> RightHandSide;
		//	RightHandSide[0] = LocalMatrix[0][3];
		//	RightHandSide[1] = LocalMatrix[1][3];
		//	RightHandSide[2] = LocalMatrix[2][3];
		//	RightHandSide[3] = LocalMatrix[3][3];
		//
		//	// Solve the equation by inverting PerspectiveMatrix and multiplying
		//	// rightHandSide by the inverse.  (This is the easiest way, not
		//	// necessarily the best.)
		//	mat<4, 4, T, Q> InversePerspectiveMatrix = glm::inverse(PerspectiveMatrix);//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
		//	mat<4, 4, T, Q> TransposedInversePerspectiveMatrix = glm::transpose(InversePerspectiveMatrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);
		//
		//	Perspective = TransposedInversePerspectiveMatrix * RightHandSide;
		//	//  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);
		//
		//	// Clear the perspective partition
		//	LocalMatrix[0][3] = LocalMatrix[1][3] = LocalMatrix[2][3] = static_cast<T>(0);
		//	LocalMatrix[3][3] = static_cast<T>(1);
		//}
		//else
		//{
		//	// No perspective.
		//	Perspective = vec<4, T, Q>(0, 0, 0, 1);
		//}

		// Next take care of translation (easy).
		translation = vec3(LocalMatrix[3]);
		LocalMatrix[3] = vec4(0, 0, 0, LocalMatrix[3].w);

		vec3 Row[3]{ vec3() };

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = ScaleFunc(Row[0], static_cast<T>(1));

		// Ignore shear
		//// Compute XY shear factor and make 2nd row orthogonal to 1st.
		//Skew.z = dot(Row[0], Row[1]);
		//Row[1] = detail::combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

		// Now, compute Y scale and normalize 2nd row.
		scale.y = length(Row[1]);
		Row[1] = ScaleFunc(Row[1], static_cast<T>(1));
		//Skew.z /= Scale.y;

		//// Compute XZ and YZ shears, orthogonalize 3rd row.
		//Skew.y = glm::dot(Row[0], Row[2]);
		//Row[2] = detail::combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
		//Skew.x = glm::dot(Row[1], Row[2]);
		//Row[2] = detail::combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

		// Next, get Z scale and normalize 3rd row.
		scale.z = length(Row[2]);
		Row[2] = ScaleFunc(Row[2], static_cast<T>(1));
		//Skew.y /= Scale.z;
		//Skew.x /= Scale.z;

#if _DEBUG
		// At this point, the matrix (in rows[]) is orthonormal.
		// Check for a coordinate system flip.  If the determinant
		// is -1, then negate the matrix and the scaling factors.
		vec3 Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
		//VX_CORE_ASSERT(dot(Row[0], Pdum3) >= static_cast<T>(0), "");
#endif
		//if (dot(Row[0], Pdum3) < 0)
		//{
		//	for (length_t i = 0; i < 3; i++)
		//	{
		//		scale[i] *= static_cast<T>(-1);
		//		Row[i] *= static_cast<T>(-1);
		//	}
		//}

		// Rotation as XYZ Euler angles
		//rotation.y = asin(-Row[0][2]);
		//if (cos(rotation.y) != 0.f)
		//{
		//	rotation.x = atan2(Row[1][2], Row[2][2]);
		//	rotation.z = atan2(Row[0][1], Row[0][0]);
		//}
		//else
		//{
		//	rotation.x = atan2(-Row[2][0], Row[1][1]);
		//	rotation.z = 0;
		//}

		// Rotation as quaternion
		int i, j, k = 0;
		T root, trace = Row[0].x + Row[1].y + Row[2].z;
		if (trace > static_cast<T>(0))
		{
			root = sqrt(trace + static_cast<T>(1));
			rotation.w = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation.x = root * (Row[1].z - Row[2].y);
			rotation.y = root * (Row[2].x - Row[0].z);
			rotation.z = root * (Row[0].y - Row[1].x);
		} // End if > 0
		else
		{
			static int Next[3] = { 1, 2, 0 };
			i = 0;
			if (Row[1].y > Row[0].x) i = 1;
			if (Row[2].z > Row[i][i]) i = 2;
			j = Next[i];
			k = Next[j];

			root = sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

			rotation[i] = static_cast<T>(0.5) * root;
			root = static_cast<T>(0.5) / root;
			rotation[j] = root * (Row[i][j] + Row[j][i]);
			rotation[k] = root * (Row[i][k] + Row[k][i]);
			rotation.w = root * (Row[j][k] - Row[k][j]);
		} // End if <= 0

		return true;
	}

}
