#include "vxpch.h"
#include "AudioUtils.h"

namespace Vortex {

	namespace Utils {

		std::string WaveDeviceTypeToString(Wave::DeviceType type)
		{
			switch (type)
			{
				case Wave::DeviceType::Playback: return "Playback";
				case Wave::DeviceType::Capture:  return "Capture";
			}

			VX_CORE_ASSERT(false, "Uknown device type!");
			return "None";
		}

	}

}
