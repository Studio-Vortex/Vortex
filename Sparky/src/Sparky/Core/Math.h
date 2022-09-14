#pragma once

#include "Sparky/Core/Base.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Sparky::Math {

	typedef glm::vec2 vec2;
	typedef glm::vec3 vec3;
	typedef glm::vec4 vec4;
	typedef glm::mat2 mat2;
	typedef glm::mat3 mat3;
	typedef glm::mat4 mat4;
	typedef glm::quat quaternion;

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
	static auto Ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		return glm::ortho<T>(left, right, bottom, top, zNear, zFar);
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
	static auto ToMat4(T v)
	{
		return glm::toMat4(v);
	}
	
	template <typename T>
	static auto Quaternion(T v)
	{
		return glm::quat(v);
	}

	static auto Inverse(const mat4& matrix)
	{
		return glm::inverse(matrix);
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

	static bool DecomposeTransform(const mat4& transform, vec3& translation, vec3& rotation, vec3& scale)
	{
		// From glm::decompose in matrix_decompose.inl

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

		vec3 Row[3], Pdum3;

		// Now get scale and shear.
		for (length_t i = 0; i < 3; ++i)
			for (length_t j = 0; j < 3; ++j)
				Row[i][j] = LocalMatrix[i][j];

		// Compute X scale factor and normalize first row.
		scale.x = length(Row[0]);
		Row[0] = detail::scale(Row[0], static_cast<T>(1));
		scale.y = length(Row[1]);
		Row[1] = detail::scale(Row[1], static_cast<T>(1));
		scale.z = length(Row[2]);
		Row[2] = detail::scale(Row[2], static_cast<T>(1));

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
		if (cos(rotation.y) != 0)
		{
			rotation.x = atan2(Row[1][2], Row[2][2]);
			rotation.z = atan2(Row[0][1], Row[0][0]);
		}
		else
		{
			rotation.x = atan2(-Row[2][0], Row[1][1]);
			rotation.z = 0;
		}

		return true;
	}

	inline std::ostream& operator<<(std::ostream& stream, const vec2& vec)
	{
		return stream << "(" << vec.x << ", " << vec.y << ')';
	}
		
	inline std::ostream& operator<<(std::ostream& stream, const vec3& vec)
	{
		return stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ')';
	}
		
	inline std::ostream& operator<<(std::ostream& stream, const vec4& vec)
	{
		return stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
	}

}