#include "vxpch.h"
#include "AudioSystem.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Module/Module.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Audio/AudioContext.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioListener.h"
#include "Vortex/Audio/AudioAssert.h"

#include "Vortex/UI/UI.h"

namespace Vortex {

	struct AudioSystemInternalData
	{
		SharedReference<AudioContext> Context = nullptr;
		AudioSystem::AudioAPI AudioAPI = AudioSystem::AudioAPI::MiniAudio;

		struct SceneAudioData
		{
			std::unordered_map<UUID, AssetHandle> ActiveAudioSources;
			std::unordered_map<UUID, AssetHandle> ActiveAudioListeners;

			std::vector<AssetHandle> PausedAudioSources;
		};

		std::unordered_map<Scene*, SceneAudioData> ActiveScenes;

		SubModule Module;
	};

	static AudioSystemInternalData* s_Data;

	AudioSystem::AudioSystem()
		: IAssetSystem("Audio System")
	{
	}

	void AudioSystem::Init()
	{
		s_Data = new AudioSystemInternalData();

		s_Data->Context = AudioContext::Create();
		s_Data->Context->Init();

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Audio";
		moduleProps.APIVersion = Version(1, 2, 0);
		moduleProps.RequiredModules = {};
		s_Data->Module.Init(moduleProps);

		Application::Get().AddModule(s_Data->Module);
	}

	void AudioSystem::Shutdown()
	{
		s_Data->ActiveScenes.clear();

		s_Data->Context->Shutdown();

		Application::Get().RemoveModule(s_Data->Module);
		s_Data->Module.Shutdown();

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
		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid Scene!");

		auto& audioData = s_Data->ActiveScenes[context];

		audioData.ActiveAudioSources.clear();
		audioData.PausedAudioSources.clear();
		audioData.ActiveAudioListeners.clear();

		s_Data->ActiveScenes.erase(context);
	}

	void AudioSystem::CreateAsset(Entity& entity, Scene* context)
	{
		CreateAsset(entity, context, "");
	}

	void AudioSystem::CreateAsset(Entity& entity, Scene* context, const std::string& filepath)
    {
		VX_PROFILE_FUNCTION();

		if (filepath.empty())
			return;

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid Scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioSourceComponent>(), "Entity doesn't have audio source component!");
		
		AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
		asc.AudioHandle = Project::GetEditorAssetManager()->GetAssetHandleFromFilepath(filepath);

		auto& audioData = s_Data->ActiveScenes[context];

		VX_CORE_ASSERT(!audioData.ActiveAudioSources.contains(entity.GetUUID()), "Entities can only have one audio source component!");

		audioData.ActiveAudioSources[entity.GetUUID()] = asc.AudioHandle;
    }

    void AudioSystem::DestroyAsset(Entity& entity, Scene* context)
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

		std::map<float, AssetHandle> audioSourceDistances;

		for (const auto entityID : view)
		{
			Entity audioSourceEntity = { entityID, context };
			const AudioSourceComponent& asc = audioSourceEntity.GetComponent<AudioSourceComponent>();

			if (!AssetManager::IsHandleValid(asc.AudioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			const uint8_t deviceListeners = asc.AudioHandle;

			if (deviceListeners >= PlaybackDevice::MaxDeviceListeners)
				continue;

			float distance = Math::Distance(entity.GetTransform().Translation, audioSourceEntity.GetTransform().Translation);
			audioSourceDistances[distance] = asc.AudioHandle;
		}

		for (const auto& [distance, audioHandle] : audioSourceDistances)
		{
			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			nextListenerIndex = audioSource->GetPlaybackDevice().GetDeviceListenerCount();
			availableAudioSource = audioSource;
			break;
		}

		AudioListenerComponent& alc = entity.GetComponent<AudioListenerComponent>();
		if (!audioSourceDistances.empty())
		{
			// TODO this should use the asset system one thats up and running
			//SharedReference<AudioListener> audioListener = Project::GetEditorAssetManager()->CreateNewAsset<AudioListener>("Audio", "", ListenerDeviceProperties{}, availableAudioSource->GetPlaybackDevice(), nextListenerIndex);
			//alc.ListenerHandle = audioListener->Handle;
		}

		// TODO fix this
		if (!AssetManager::IsHandleValid(alc.ListenerHandle))
			return;

		auto& audioData = s_Data->ActiveScenes[context];
		VX_CORE_ASSERT(!audioData.ActiveAudioListeners.contains(entity.GetUUID()), "Entities can only have one audio listener component!");

		audioData.ActiveAudioListeners[entity.GetUUID()] = alc.ListenerHandle;
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

    void AudioSystem::OnRuntimeStart(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		const auto& audioData = s_Data->ActiveScenes[context];

		for (const auto& [entityUUID, audioHandle] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			if (!AssetManager::IsHandleValid(audioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			if (!audioSource)
				continue;

			if (audioSource->IsPlaying())
				audioSource->Stop();

			const auto& audioProps = audioSource->GetProperties();
			if (!audioProps.PlayOnStart)
				continue;

			audioSource->Play();
		}
	}

	void AudioSystem::OnUpdateRuntime(Scene* context)
	{
	}

	void AudioSystem::OnRuntimeScenePaused(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		auto& audioData = s_Data->ActiveScenes[context];
		auto& pausedAudioSources = audioData.PausedAudioSources;

		for (const auto& [entityUUID, audioHandle] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);
			if (!entity.IsActive())
				continue;

			if (!AssetManager::IsHandleValid(audioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			if (!audioSource)
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Pause();
			pausedAudioSources.push_back(audioSource->Handle);
		}
	}

	void AudioSystem::OnRuntimeSceneResumed(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(context->IsRunning(), "Scene must be running!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();
		if (projectProps.EditorProps.MuteAudioSources)
			return;

		auto& audioData = s_Data->ActiveScenes[context];

		for (const auto& audioHandle : audioData.PausedAudioSources)
		{
			if (!AssetManager::IsHandleValid(audioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			if (!audioSource)
				continue;

			audioSource->Play();
		}

		audioData.PausedAudioSources.clear();
	}

	void AudioSystem::OnRuntimeStop(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");
		VX_CORE_ASSERT(s_Data->ActiveScenes.contains(context), "Invalid scene!");

		const auto& audioData = s_Data->ActiveScenes[context];

		for (const auto& [entityUUID, audioHandle] : audioData.ActiveAudioSources)
		{
			Entity entity = context->TryGetEntityWithUUID(entityUUID);

			if (!AssetManager::IsHandleValid(audioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			if (!audioSource)
				continue;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Stop();
		}
	}

	AudioSystem::AudioAPI AudioSystem::GetAudioAPI()
	{
		return s_Data->AudioAPI;
	}

	SharedReference<AudioContext> AudioSystem::GetAudioContext()
	{
		return s_Data->Context;
	}

	void AudioSystem::OnGuiRender()
	{
	}

}
