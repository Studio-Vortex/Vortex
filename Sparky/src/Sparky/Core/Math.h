#pragma once

#include "Sparky/Core/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Sparky::Math {

	typedef glm::vec2 vec2;
	typedef glm::vec3 vec3;
	typedef glm::vec4 vec4;
	typedef glm::mat2 mat2;
	typedef glm::mat3 mat3;
	typedef glm::mat4 mat4;

	static auto Deg2Rad(float degrees)
	{
		return glm::radians(degrees);
	}

	static auto Rad2Deg(float radians)
	{
		return glm::degrees(radians);
	}

	template <typename T>
	static auto Ortho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		return glm::ortho(left, right, bottom, top, zNear, zFar);
	}

	static auto Translate(const mat4& matrix, const vec3& translation)
	{
		return glm::translate(matrix, translation);
	}

	static auto Rotate(const mat4& matrix, float rotation, const vec3& axis)
	{
		return glm::rotate(matrix, glm::radians(rotation), axis);
	}

	static auto Scale(const mat4& matrix, const vec3& scale)
	{
		return glm::scale(matrix, scale);
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