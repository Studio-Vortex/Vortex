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

		ma_uint32 CaptureDeviceCount;
		ma_device_info* CaptureDeviceInfos = nullptr;

#endif // !VX_DIST

	};

	static MiniAudioContextInternalData s_Data;

	void MiniAudioContext::Init()
	{
		ma_context_config config = ma_context_config_init();
		config.pUserData = this;

		ma_result res = ma_context_init(nullptr, 0, &config, &s_Data.Context);
		VX_CHECK_AUDIO_RESULT(res, "Failed to initialize miniaudio!");

#ifndef VX_DIST

		EnumerateDevices();

#endif // !VX_DIST

	}

	void MiniAudioContext::Shutdown()
	{
		ma_result res = ma_context_uninit(&s_Data.Context);
		VX_CHECK_AUDIO_RESULT(res, "Failed to shutdown miniaudio!");
	}

    void* MiniAudioContext::GetLogger()
    {
        return (void*)ma_context_get_log(&s_Data.Context);
    }

	void MiniAudioContext::EnumerateDevices()
	{
		ma_result res = ma_context_get_devices(
			&s_Data.Context,
			&s_Data.PlaybackDeviceInfos,
			&s_Data.PlaybackDeviceCount,
			&s_Data.CaptureDeviceInfos,
			&s_Data.CaptureDeviceCount
		);
		VX_CHECK_AUDIO_RESULT(res, "Failed to enumerate hardware devices!");

		VX_CONSOLE_LOG_INFO("[Audio] Located '{}' playback device(s)", s_Data.PlaybackDeviceCount);
		VX_CONSOLE_LOG_INFO("[Audio] Located '{}' capture device(s)", s_Data.CaptureDeviceCount);

		uint32_t defaultDevice = 0;

		{
			for (uint32_t i = 0; i < s_Data.PlaybackDeviceCount; i++)
			{
				VX_CONSOLE_LOG_INFO("[Audio]	Playback-Device {}: {}", i + 1, s_Data.PlaybackDeviceInfos[i].name);
				if (!s_Data.PlaybackDeviceInfos[i].isDefault)
					continue;

				defaultDevice = i;
			}

			VX_CONSOLE_LOG_INFO("[Audio] Default Playback-Device: {}", s_Data.PlaybackDeviceInfos[defaultDevice].name);
		}

		{
			for (uint32_t i = 0; i < s_Data.CaptureDeviceCount; i++)
			{
				VX_CONSOLE_LOG_INFO("[Audio]	Capture-Device {}: {}", i + 1, s_Data.CaptureDeviceInfos[i].name);
				if (!s_Data.CaptureDeviceInfos[i].isDefault)
					continue;

				defaultDevice = i;
			}

			VX_CONSOLE_LOG_INFO("[Audio] Default Capture-Device: {}", s_Data.CaptureDeviceInfos[defaultDevice].name);
		}
	}

}
