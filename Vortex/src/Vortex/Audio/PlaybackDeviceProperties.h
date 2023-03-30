#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Audio/AudioCone.h"
#include "Vortex/Audio/AttenuationModel.h"

namespace Vortex {

	struct VORTEX_API PlaybackDeviceProperties
	{
		Math::vec3 Position = Math::vec3(0.0f);
		Math::vec3 Direction = Math::vec3(0.0f);
		Math::vec3 Velocity = Math::vec3(0.0f);

		AudioCone Cone;
		float MinGain = 1.0f;
		float MaxGain = 1.0f;
		AttenuationModel AttenuationModel = AttenuationModel::Inverse;
		float Falloff = 1.0f;
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