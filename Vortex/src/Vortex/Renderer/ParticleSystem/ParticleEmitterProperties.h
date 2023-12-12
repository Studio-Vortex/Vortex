#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Math/Math.h"

namespace Vortex {

	struct VORTEX_API ParticleEmitterProperties
	{
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Offset = Math::vec3(0.0f);
		Math::vec3 Velocity = Math::vec3(1.0f);
		Math::vec3 VelocityVariation = Math::vec3(1.0f);
		Math::vec2 SizeBegin = Math::vec2(1.0f);
		Math::vec2 SizeEnd = Math::vec2(1.0f);
		Math::vec2 SizeVariation = Math::vec2(1.0f);
		Math::vec4 ColorBegin = Math::vec4(1.0f);
		Math::vec4 ColorEnd = Math::vec4(1.0f);

		float Rotation = 0.1f;
		float LifeTime = 1.0f;

		bool GenerateRandomColors = false;
	};

}
