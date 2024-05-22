#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"

#include <cstdint>

namespace Vortex {

	struct VORTEX_API Vertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec3 Normal;
		Math::vec3 Tangent;
		Math::vec3 BiTangent;
		Math::vec2 TexCoord;
		Math::vec2 TexScale;
		Math::ivec4 BoneIDs;
		Math::vec4 BoneWeights;

		// Editor-only
		int EntityID;
	};

	struct VORTEX_API StaticVertex
	{
		Math::vec3 Position;
		Math::vec4 Color;
		Math::vec3 Normal;
		Math::vec3 Tangent;
		Math::vec3 BiTangent;
		Math::vec2 TexCoord;
		Math::vec2 TexScale;

		// Editor-only
		int EntityID;
	};

	struct VORTEX_API VertexIndex
	{
		uint32_t I0, I1, I2;
	};

}
