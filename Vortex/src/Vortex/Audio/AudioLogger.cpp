#include "vxpch.h"
#include "AudioLogger.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

	void AudioLogger::AudioLogCallback(void* pUserData, uint32_t logLevel, const char* pMessage)
	{
		std::string message(pMessage);
		message.shrink_to_fit();

		switch ((ma_log_level)logLevel)
		{
			//case MA_LOG_LEVEL_INFO:    VX_CONSOLE_LOG_INFO("[Audio] {}", message);  break;
			case MA_LOG_LEVEL_WARNING: VX_CONSOLE_LOG_WARN("[Audio] {}", message);  break;
			case MA_LOG_LEVEL_ERROR:   VX_CONSOLE_LOG_ERROR("[Audio] {}", message); break;
			case MA_LOG_LEVEL_DEBUG:   VX_CONSOLE_LOG_FATAL("[Audio] {}", message); break;
		}
	}

}
