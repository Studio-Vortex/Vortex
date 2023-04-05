#pragma once

namespace Vortex {

	class AudioLogger
	{
	public:
		static void AudioLogCallback(void* pUserData, uint32_t logLevel, const char* pMessage);
	};

}
