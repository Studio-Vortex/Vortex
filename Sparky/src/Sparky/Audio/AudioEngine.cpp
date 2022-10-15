#include "sppch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Sparky {

	struct AudioEngineInternalData
	{
		ma_engine Engine;
		ma_result Result;
		ma_decoder Decoder;
		ma_device_config DeviceConfig;
		ma_device Device;
		ma_context Context;
		ma_device_info* pPlaybackDeviceInfos;
		ma_uint32 PlaybackDeviceCount;
		ma_device_info* pCaptureDeviceInfos;
		ma_uint32 CaptureDeviceCount;
	};

	static AudioEngineInternalData s_Data;

	static void AudioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
	{
		ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
		if (pDecoder == NULL) {
			return;
		}

		/* Reading PCM frames will loop based on what we specified when called ma_data_source_set_looping(). */
		ma_data_source_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

		(void)pInput;
	}

	void AudioEngine::Init()
	{
		s_Data.Result = ma_context_init(NULL, 0, NULL, &s_Data.Context);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize Audio Context!");

		s_Data.Result = ma_context_get_devices(&s_Data.Context, &s_Data.pPlaybackDeviceInfos, &s_Data.PlaybackDeviceCount, &s_Data.pCaptureDeviceInfos, &s_Data.CaptureDeviceCount);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to retrieve Audio Device Information!");

		SP_CORE_INFO("Playback Devices");
		for (uint32_t i = 0; i < s_Data.PlaybackDeviceCount; ++i)
			SP_CORE_INFO("  {}: {}", i, s_Data.pPlaybackDeviceInfos[i].name);

		s_Data.Result = ma_engine_init(nullptr, &s_Data.Engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "AudioEngine failed to initialize!");
	}

	void AudioEngine::Shutdown()
	{
		ma_decoder_uninit(&s_Data.Decoder);
		ma_context_uninit(&s_Data.Context);
		ma_device_uninit(&s_Data.Device);
		ma_engine_uninit(&s_Data.Engine);
	}

	void AudioEngine::PlayFromAudioSource(const SharedRef<AudioSource>& audioSource)
	{
		const AudioSource::AudioProperties& audioProperties = audioSource->GetProperties();
		const std::string& audioSourcePath = audioSource->GetPath();

		if (audioProperties.Loop)
		{
			s_Data.Result = ma_decoder_init_file(audioSourcePath.c_str(), nullptr, &s_Data.Decoder);
			SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to decode file from " + audioSourcePath);

			ma_data_source_set_looping(&s_Data.Decoder, MA_TRUE);

			s_Data.DeviceConfig = ma_device_config_init(ma_device_type_playback);
			s_Data.DeviceConfig.playback.format = s_Data.Decoder.outputFormat;
			s_Data.DeviceConfig.playback.channels = s_Data.Decoder.outputChannels;
			s_Data.DeviceConfig.sampleRate = s_Data.Decoder.outputSampleRate;
			s_Data.DeviceConfig.dataCallback = AudioDataCallback;
			s_Data.DeviceConfig.pUserData = &s_Data.Decoder;

			s_Data.Result = ma_device_init(nullptr, &s_Data.DeviceConfig, &s_Data.Device);
			SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize Playback Device!");

			s_Data.Result = ma_device_start(&s_Data.Device);
			SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to start Audio Playback Device!");
		}
		else
		{
			// If the path doesn't exist and we try to play the non-existant file the engine will crash
			if (std::filesystem::exists(audioSourcePath))
			{
				s_Data.Result = ma_engine_play_sound(&s_Data.Engine, audioSourcePath.c_str(), nullptr);
				SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to play audio from " + audioSourcePath);
			}
		}
	}

	void AudioEngine::StartAllAudio()
	{
		s_Data.Result = ma_engine_start(&s_Data.Engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to start all audio!");
	}

	void AudioEngine::StopAllAudio()
	{
		s_Data.Result = ma_engine_stop(&s_Data.Engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to stop all audio!");
	}

}
