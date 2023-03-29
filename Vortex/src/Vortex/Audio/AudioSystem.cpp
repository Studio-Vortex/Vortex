#include "vxpch.h"
#include "AudioSystem.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

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

	void AudioSystem::StartAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();

		auto view = contextScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };

			if (!entity.IsActive())
				continue;

			SharedReference<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			const auto& audioProps = audioSource->GetProperties();

			if (audioSource->IsPlaying())
				audioSource->Stop();

			if (!audioProps.PlayOnStart)
				continue;

			audioSource->Play();
		}
	}

	void AudioSystem::PauseAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(contextScene->IsRunning(), "Scene must be running!");

		const auto view = contextScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			SharedReference<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

			if (!entity.IsActive())
				continue;

			if (!audioSource)
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Pause();
			s_AudioSourcesToResume.push_back(audioSource);
		}
	}

	void AudioSystem::ResumeAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(contextScene->IsRunning(), "Scene must be running!");

		SharedReference<Project> activeProject = Project::GetActive();
		ProjectProperties projectProps = activeProject->GetProperties();

		if (projectProps.EditorProps.MuteAudioSources)
			return;

		for (auto& audioSource : s_AudioSourcesToResume)
			audioSource->Play();

		s_AudioSourcesToResume.clear();
	}

	void AudioSystem::StopAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();

		auto view = contextScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			SharedReference<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Stop();
		}
	}

}
