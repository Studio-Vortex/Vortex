#include "vxpch.h"
#include "AudioLogger.h"

#include <Wave/Wave.h>

#include <string>

namespace Vortex {

	void AudioLogger::DebugLogCallback(void* pUserData, uint32_t logLevel, const char* pMessage)
	{
		std::string message(pMessage);
		message.shrink_to_fit();

		using namespace Wave;

		switch ((LogLevel)logLevel)
		{
			case LogLevel::Info:    VX_CONSOLE_LOG_INFO("[Audio] {}", message);  return;
			case LogLevel::Warning: VX_CONSOLE_LOG_WARN("[Audio] {}", message);  return;
			case LogLevel::Error:   VX_CONSOLE_LOG_ERROR("[Audio] {}", message); return;
			case LogLevel::Debug:   VX_CONSOLE_LOG_FATAL("[Audio] {}", message); return;
		}
	}

}
