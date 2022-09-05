#pragma once

#include "Sparky/Core.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Sparky {

	namespace Math {

		typedef glm::vec2 vec2;
		typedef glm::vec3 vec3;
		typedef glm::vec4 vec4;
		typedef glm::mat2 mat2;
		typedef glm::mat3 mat3;
		typedef glm::mat4 mat4;

		inline std::ostream& operator<<(std::ostream& stream, const vec2& vec)
		{
			return stream << "Vector2: (" << vec.x << ", " << vec.y << ')';
		}
		
		inline std::ostream& operator<<(std::ostream& stream, const vec3& vec)
		{
			return stream << "Vector3: (" << vec.x << ", " << vec.y << ", " << vec.z << ')';
		}
		
		inline std::ostream& operator<<(std::ostream& stream, const vec4& vec)
		{
			return stream << "Vector4: (" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ')';
		}

	}

}