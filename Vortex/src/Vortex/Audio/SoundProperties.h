#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

namespace Vortex {

	struct VORTEX_API SoundProperties
	{
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Direction = Math::vec3(0.0f);
		Math::vec3 Velocity = Math::vec3(0.0f);

		struct VORTEX_API AudioCone
		{
			float InnerAngle = Math::Deg2Rad(10.0f);
			float OuterAngle = Math::Deg2Rad(45.0f);
			float OuterGain = 0.0f;
		} Cone;

		float MinDistance = 1.0f;
		float MaxDistance = 10.0f;

		float Pitch = 1.0f;
		float DopplerFactor = 1.0f;
		float Volume = 1.0f;

		bool PlayOnStart = false;
		bool PlayOneShot = false;
		bool Loop = false;
		bool Spacialized = true;
	};

}
