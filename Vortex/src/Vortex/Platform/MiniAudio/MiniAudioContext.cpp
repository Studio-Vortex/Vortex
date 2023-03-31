#include "vxpch.h"
#include "MiniAudioContext.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include "Vortex/Audio/AudioAssert.h"

namespace Vortex {

	struct MiniAudioContextInternalData
	{
		ma_context Context;

#ifndef VX_DIST

		ma_uint32 PlaybackDeviceCount;
		ma_device_info* PlaybackDeviceInfos = nullptr;

#endif // !VX_DIST

	};

	static MiniAudioContextInternalData s_Data;

	void MiniAudioContext::Init()
	{
		ma_context_config contextConfig = ma_context_config_init();
		contextConfig.pUserData = this;

		VX_CHECK_AUDIO_RESULT(
			ma_context_init(NULL, 0, &contextConfig, &s_Data.Context),
			"Failed to initialize Audio Context!"
		);

#ifndef VX_DIST

		EnumerateDevices();

#endif // !VX_DIST
	}

	void MiniAudioContext::Shutdown()
	{
		VX_CHECK_AUDIO_RESULT(
			ma_context_uninit(&s_Data.Context),
			"Failed to shutdown audio context!"
		);
	}

    void* MiniAudioContext::GetLogger()
    {
        return ma_context_get_log(&s_Data.Context);
    }

	void MiniAudioContext::EnumerateDevices()
	{
		VX_CHECK_AUDIO_RESULT(
			ma_context_get_devices(&s_Data.Context, &s_Data.PlaybackDeviceInfos, &s_Data.PlaybackDeviceCount, nullptr, nullptr),
			"Failed to retrieve Audio Hardware Information!"
		);

		const bool hasMultipleDevices = s_Data.PlaybackDeviceCount > 1;
		VX_CONSOLE_LOG_INFO("[Audio] Located {} hardware device{}", s_Data.PlaybackDeviceCount, hasMultipleDevices ? "(s)" : "");

		for (uint32_t i = 0; i < s_Data.PlaybackDeviceCount; i++)
		{
			VX_CONSOLE_LOG_INFO("[Audio] Device {}: {}", i + 1, s_Data.PlaybackDeviceInfos[i].name);
			VX_CONSOLE_LOG_INFO("[Audio]        Default - {}", s_Data.PlaybackDeviceInfos[i].isDefault ? "true" : "false");
		}
	}

}
