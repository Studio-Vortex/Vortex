#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Math/Math.h"

#include "Vortex/Audio/AudioTypes.h"

#include <Wave/Wave.h>

#include <string>

namespace Vortex {

	namespace Utils {

		std::string WaveDeviceTypeToString(Wave::DeviceType type);

		static inline Math::vec3 FromWaveVector(const Wave::Vec3& vec) { return *(Math::vec3*)&vec; }
		static inline Wave::Vec3 ToWaveVector(const Math::vec3& vec) { return *(Wave::Vec3*)&vec; }

		static inline AttenuationModel FromWaveAttenuationModel(Wave::AttenuationModel model) { return (AttenuationModel)model; }
		static inline Wave::AttenuationModel ToWaveAttenuationModel(AttenuationModel model) { return (Wave::AttenuationModel)model; }

		static inline PanMode FromWavePanMode(Wave::PanMode mode) { return (PanMode)mode; }
		static inline Wave::PanMode ToWavePanMode(PanMode mode) { return (Wave::PanMode)mode; }

		static inline PositioningMode FromWavePositioningMode(Wave::Positioning mode) { return (PositioningMode)mode; }
		static inline Wave::Positioning ToWavePositioningMode(PositioningMode mode) { return (Wave::Positioning)mode; }

	}

}
