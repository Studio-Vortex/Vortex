#include "vxpch.h"
#include "AudioSystem.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Vortex {

	struct AudioSystemInternalData
	{
		ma_result Result;
		ma_context Context;
		ma_uint32 PlaybackDeviceCount;
		ma_device_info* pPlaybackDeviceInfos;
	};

	static AudioSystemInternalData s_Data;

	void AudioSystem::Init()
	{
		s_Data.Result = ma_context_init(NULL, 0, NULL, &s_Data.Context);
		VX_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize Audio Context!");

		s_Data.Result = ma_context_get_devices(&s_Data.Context, &s_Data.pPlaybackDeviceInfos, &s_Data.PlaybackDeviceCount, nullptr, nullptr);
		VX_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to retrieve Audio Device Information!");

		VX_CORE_INFO_TAG("Audio", "Audio Engine Located {} device(s)", s_Data.PlaybackDeviceCount);
		for (uint32_t i = 0; i < s_Data.PlaybackDeviceCount; ++i)
		{
			VX_CORE_INFO_TAG("Audio", "  {}: {}", i + 1, s_Data.pPlaybackDeviceInfos[i].name);
		}
	}

	void AudioSystem::Shutdown()
	{
		ma_context_uninit(&s_Data.Context);
	}

}
