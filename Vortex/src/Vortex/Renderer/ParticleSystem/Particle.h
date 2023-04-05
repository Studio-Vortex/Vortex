#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	struct VORTEX_API Particle
	{
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Velocity = Math::vec3(1.0f);
		Math::vec2 SizeBegin = Math::vec2(1.0f), SizeEnd = Math::vec2(1.0f);
		Math::vec4 ColorBegin = Math::vec4(1.0f), ColorEnd = Math::vec4(1.0f);
		Math::vec4 RandomColor = Math::vec4(1.0f);
		float Rotation = 0.0f;

		float LifeTime = 1.0f;
		float LifeRemaining = 0.0f;

		bool Active = false;
	};

}
