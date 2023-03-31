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
		ma_context AudioContext;
		
#ifndef VX_DIST

		ma_uint32 PlaybackDeviceCount;
		ma_device_info* PlaybackDeviceInfos = nullptr;

#endif // !VX_DIST

		struct SceneAudioData
		{
			std::unordered_map<UUID, SharedReference<AudioSource>> ActiveAudioSources;
			std::unordered_map<UUID, SharedReference<AudioListener>> ActiveAudioListeners;

			std::vector<SharedReference<AudioSource>> PausedAudioSources;
		};

		std::unordered_map<Scene*, SceneAudioData> ActiveScenes;
	};

	static AudioSystemInternalData* s_Data;

	void AudioSystem::Init()
	{
		s_Data = new AudioSystemInternalData();

		VX_CHECK_AUDIO_RESULT(
			ma_context_init(NULL, 0, NULL, &s_Data->AudioContext),
			"Failed to initialize Audio Context!"
		);

#ifndef VX_DIST
		
		VX_CHECK_AUDIO_RESULT(
			ma_context_get_devices(&s_Data->AudioContext, &s_Data->PlaybackDeviceInfos, &s_Data->PlaybackDeviceCount, nullptr, nullptr),
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
		s_Data->ActiveScenes.clear();

		VX_CHECK_AUDIO_RESULT(
			ma_context_uninit(&s_Data->AudioContext),
			"Failed to shutdown audio context!"
		);

		delete s_Data;
		s_Data = nullptr;
	}

	void AudioSystem::SubmitContextScene(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(!s_Data->ActiveScenes.contains(context), "Scene was already added to audio system!");
		
		s_Data->ActiveScenes[context] = AudioSystemInternalData::SceneAudioData{};
	}

	void AudioSystem::RemoveContextScene(Scene* context)
	{
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid Scene!");

		auto& audioData = s_Data->ActiveScenes[context];

		audioData.ActiveAudioSources.clear();
		audioData.PausedAudioSources.clear();
		audioData.ActiveAudioListeners.clear();

		s_Data->ActiveScenes.erase(context);
	}

    void AudioSystem::CreateAudioSource(Entity& entity, Scene* context, const std::string& filepath)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid Scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioSourceComponent>(), "Entity doesn't have audio source component!");
		
		AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();

		if (filepath.empty())
			asc.Source = AudioSource::Create();
		else
			asc.Source = AudioSource::Create(filepath);

		VX_CORE_ASSERT(asc.Source, "Failed to create audio source!");

		auto& audioData = s_Data->ActiveScenes[context];

		VX_CORE_ASSERT(!audioData.ActiveAudioSources.contains(entity.GetUUID()), "Entities can only have one audio source component!");

		audioData.ActiveAudioSources[entity.GetUUID()] = asc.Source;
    }

    void AudioSystem::DestroyAudioSource(Entity& entity, Scene* context)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid Scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioSourceComponent>(), "Entity doesn't have audio source component!");

		auto& audioData = s_Data->ActiveScenes[context];
		VX_CORE_ASSERT(audioData.ActiveAudioSources.contains(entity.GetUUID()), "Entity was not found in scene audio data map!");

		audioData.ActiveAudioSources.erase(entity.GetUUID());
    }

    void AudioSystem::CreateAudioListener(Entity& entity, Scene* context)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioListenerComponent>(), "Entity doesn't have audio listener component!");

		// TODO handle listener index here?
		uint8_t nextListenerIndex;
		SharedReference<AudioSource> availableAudioSource;

		auto view = context->GetAllEntitiesWith<AudioSourceComponent>();

		std::map<float, SharedReference<AudioSource>> audioSourceDistances;

		for (const auto& entityID : view)
		{
			Entity audioSourceEntity = { entityID, context };
			const AudioSourceComponent& audioSourceComponent = audioSourceEntity.GetComponent<AudioSourceComponent>();
			const uint8_t deviceListeners = audioSourceComponent.Source->GetPlaybackDevice().GetDeviceListenerCount();

			if (deviceListeners >= PlaybackDevice::MaxDeviceListeners)
				continue;

			float distance = Math::Distance(entity.GetTransform().Translation, audioSourceEntity.GetTransform().Translation);
			audioSourceDistances[distance] = audioSourceComponent.Source;
		}

		for (const auto& [distance, audioSource] : audioSourceDistances)
		{
			nextListenerIndex = audioSource->GetPlaybackDevice().GetDeviceListenerCount();
			availableAudioSource = audioSource;
			break;
		}

		AudioListenerComponent& alc = entity.GetComponent<AudioListenerComponent>();
		if (!audioSourceDistances.empty())
			alc.Listener = AudioListener::Create(ListenerDeviceProperties{}, availableAudioSource->GetPlaybackDevice(), nextListenerIndex);

		VX_CORE_ASSERT(alc.Listener, "Failed to create audio listener!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		// TODO fix this
		if (!alc.Listener)
			return;

		auto& audioData = s_Data->ActiveScenes[context];
		VX_CORE_ASSERT(!audioData.ActiveAudioListeners.contains(entity.GetUUID()), "Entities can only have one audio listener component!");

		audioData.ActiveAudioListeners[entity.GetUUID()] = alc.Listener;
    }

    void AudioSystem::DestroyAudioListener(Entity& entity, Scene* context)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioListenerComponent>(), "Entity doesn't have audio listener component!");

		auto& audioData = s_Data->ActiveScenes[context];
		VX_CORE_ASSERT(audioData.ActiveAudioListeners.contains(entity.GetUUID()), "Entity was not found in scene audio data map!");

		audioData.ActiveAudioListeners.erase(entity.GetUUID());
    }

    void AudioSystem::StartAudioSourcesRuntime(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		auto& audioData = s_Data->ActiveScenes[context];

		for (auto& [entityUUID, audioSource] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			const auto& audioProps = audioSource->GetProperties();

			if (audioSource->IsPlaying())
				audioSource->Stop();

			if (!audioProps.PlayOnStart)
				continue;

			audioSource->Play();
		}
	}

	void AudioSystem::PauseAudioSourcesRuntime(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		auto& audioData = s_Data->ActiveScenes[context];
		auto& pausedAudioSources = audioData.PausedAudioSources;

		for (auto& [entityUUID, audioSource] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Pause();
			pausedAudioSources.push_back(audioSource);
		}
	}

	void AudioSystem::ResumeAudioSourcesRuntime(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();
		if (projectProps.EditorProps.MuteAudioSources)
			return;

		auto& pausedAudioSources = s_Data->ActiveScenes[context].PausedAudioSources;

		for (auto& audioSource : pausedAudioSources)
			audioSource->Play();

		pausedAudioSources.clear();
	}

	void AudioSystem::StopAudioSourcesRuntime(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		auto& audioData = s_Data->ActiveScenes[context];

		for (auto& [entityUUID, audioSource] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Stop();
		}
	}

}
