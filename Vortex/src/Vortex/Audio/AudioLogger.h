#pragma once

#include <cstdint>

namespace Vortex {

	class AudioLogger
	{
	public:
		static void DebugLogCallback(void* pUserData, uint32_t logLevel, const char* pMessage);
	};

}
