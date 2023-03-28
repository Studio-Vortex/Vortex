#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Audio/AudioCone.h"

namespace Vortex {

	struct VORTEX_API ListenerDeviceProperties
	{
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Direction = Math::vec3(0.0f);
		Math::vec3 Veloctiy = Math::vec3(0.0f);

		AudioCone Cone;

		uint32_t ListenerIndex = 0;
	};

}
