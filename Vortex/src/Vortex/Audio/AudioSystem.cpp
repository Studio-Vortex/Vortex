#include "vxpch.h"
#include "AudioSystem.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioListener.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include "Vortex/Audio/AudioAssert.h"

namespace Vortex {

	struct AudioSystemInternalData
	{
		ma_context Context;
		
#ifndef VX_DIST

		ma_uint32 PlaybackDeviceCount;
		ma_device_info* PlaybackDeviceInfos = nullptr;

#endif // !VX_DIST

		std::unordered_map<UUID, SharedReference<AudioSource>> ActiveAudioSources;
		std::unordered_map<UUID, SharedReference<AudioListener>> ActiveAudioListeners;

		std::vector<SharedReference<AudioSource>> PausedAudioSources;
	};

	static AudioSystemInternalData* s_Data;

	void AudioSystem::Init()
	{
		s_Data = new AudioSystemInternalData();

		VX_CHECK_AUDIO_RESULT(
			ma_context_init(NULL, 0, NULL, &s_Data->Context),
			"Failed to initialize Audio Context!"
		);

#ifndef VX_DIST
		
		VX_CHECK_AUDIO_RESULT(
			ma_context_get_devices(&s_Data->Context, &s_Data->PlaybackDeviceInfos, &s_Data->PlaybackDeviceCount, nullptr, nullptr),
			"Failed to retrieve Audio Hardware Information!"
		);
		
		const bool hasMultipleDevices = s_Data->PlaybackDeviceCount > 1;
		VX_CONSOLE_LOG_INFO("[Audio] Located {} hardware device{}", s_Data->PlaybackDeviceCount, hasMultipleDevices ? "(s)" : "");

		for (uint32_t i = 0; i < s_Data->PlaybackDeviceCount; i++)
		{
			VX_CONSOLE_LOG_INFO("[Audio] Device {}: {}", i + 1, s_Data->PlaybackDeviceInfos[i].name);
			VX_CONSOLE_LOG_INFO("[Audio]        Default - {}", s_Data->PlaybackDeviceInfos[i].isDefault ? "true" : "false");
		}

#endif // !VX_DIST

	}

	void AudioSystem::Shutdown()
	{
		VX_CHECK_AUDIO_RESULT(
			ma_context_uninit(&s_Data->Context),
			"Failed to shutdown audio context!"
		);

		delete s_Data;
		s_Data = nullptr;
	}

	void AudioSystem::StartAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(contextScene->IsRunning(), "Scene must be running!");

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

		auto view = contextScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			if (!entity.IsActive())
				continue;

			SharedReference<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			if (!audioSource)
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Pause();
			s_Data->PausedAudioSources.emplace_back(audioSource);
		}
	}

	void AudioSystem::ResumeAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(contextScene->IsRunning(), "Scene must be running!");

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		if (projectProps.EditorProps.MuteAudioSources)
			return;

		for (auto& audioSource : s_Data->PausedAudioSources)
			audioSource->Play();

		s_Data->PausedAudioSources.clear();
	}

	void AudioSystem::StopAudioSources(Scene* contextScene)
	{
		VX_PROFILE_FUNCTION();

		auto view = contextScene->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			
			SharedReference<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			if (!audioSource)
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Stop();
		}
	}

}
