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
			std::unordered_map<UUID, AssetHandle> AudioSources;
			std::unordered_map<UUID, AssetHandle> AudioListeners;

			std::vector<AssetHandle> PausedAudioSources;

			void Clear()
			{
				AudioSources.clear();
				AudioListeners.clear();
				PausedAudioSources.clear();
			}
		};

		Scene* ActiveScene = nullptr;
		SceneAudioData AudioData;

		SubModule Module;
	};

	static AudioSystemInternalData s_Data;

	AudioSystem::AudioSystem()
		: IAssetSystem("Audio System")
	{
	}

	void AudioSystem::Init()
	{
		s_Data.Context = AudioContext::Create();
		s_Data.Context->Init();

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Audio";
		moduleProps.APIVersion = Version(1, 2, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void AudioSystem::Shutdown()
	{
		s_Data.AudioData.Clear();

		s_Data.Context->Shutdown();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void AudioSystem::SubmitContextScene(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
		
		s_Data.ActiveScene = context;
		s_Data.AudioData.Clear();
	}

	void AudioSystem::RemoveContextScene()
	{
		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");

		s_Data.ActiveScene = nullptr;
		s_Data.AudioData.Clear();
	}

	void AudioSystem::CreateAsset(Entity& entity)
	{
		CreateAsset(entity, "");
	}

	void AudioSystem::CreateAsset(Entity& entity, const std::string& filepath)
    {
		VX_PROFILE_FUNCTION();

		if (filepath.empty())
			return;

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid Scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioSourceComponent>(), "Entity doesn't have audio source component!");
		
		AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
		std::string name = FileSystem::RemoveFileExtension(filepath);
		std::string filename = name + ".vsound";
		SharedReference<AudioSource> asset = Project::GetEditorAssetManager()->CreateNewAsset<AudioSource>("Audio", filename, filepath);

		if (asset)
		{
			asc.AudioHandle = asset->Handle;

			VX_CORE_ASSERT(!s_Data.AudioData.AudioSources.contains(entity.GetUUID()), "Entities can only have one audio source component!");

			s_Data.AudioData.AudioSources[entity.GetUUID()] = asc.AudioHandle;
		}
    }

    void AudioSystem::DestroyAsset(Entity& entity)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid Scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioSourceComponent>(), "Entity doesn't have audio source component!");

		VX_CORE_ASSERT(s_Data.AudioData.AudioSources.contains(entity.GetUUID()), "Entity was not found in scene audio data map!");

		s_Data.AudioData.AudioSources.erase(entity.GetUUID());
    }

    void AudioSystem::CreateAudioListener(Entity& entity)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioListenerComponent>(), "Entity doesn't have audio listener component!");

		// TODO handle listener index here?
		uint8_t nextListenerIndex;
		SharedReference<AudioSource> availableAudioSource;

		auto view = s_Data.ActiveScene->GetAllEntitiesWith<AudioSourceComponent>();

		std::map<float, AssetHandle> audioSourceDistances;

		for (const auto entityID : view)
		{
			Entity audioSourceEntity = { entityID, s_Data.ActiveScene };
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

		VX_CORE_ASSERT(!s_Data.AudioData.AudioListeners.contains(entity.GetUUID()), "Entities can only have one audio listener component!");

		s_Data.AudioData.AudioListeners[entity.GetUUID()] = alc.ListenerHandle;
    }

    void AudioSystem::DestroyAudioListener(Entity& entity)
    {
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(entity.HasComponent<AudioListenerComponent>(), "Entity doesn't have audio listener component!");

		VX_CORE_ASSERT(s_Data.AudioData.AudioListeners.contains(entity.GetUUID()), "Entity was not found in scene audio data map!");

		s_Data.AudioData.AudioListeners.erase(entity.GetUUID());
    }

    void AudioSystem::OnRuntimeStart()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScene->IsRunning(), "Scene must be running!");

		for (const auto& [entityUUID, audioHandle] : s_Data.AudioData.AudioSources)
		{
			Entity entity = s_Data.ActiveScene->TryGetEntityWithUUID(entityUUID);
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

	void AudioSystem::OnUpdateRuntime()
	{
	}

	void AudioSystem::OnRuntimeScenePaused()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScene->IsRunning(), "Scene must be running!");

		for (const auto& [entityUUID, audioHandle] : s_Data.AudioData.AudioSources)
		{
			Entity entity = s_Data.ActiveScene->TryGetEntityWithUUID(entityUUID);
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
			s_Data.AudioData.PausedAudioSources.push_back(audioSource->Handle);
		}
	}

	void AudioSystem::OnRuntimeSceneResumed()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();
		if (projectProps.EditorProps.MuteAudioSources)
			return;

		for (const auto& audioHandle : s_Data.AudioData.PausedAudioSources)
		{
			if (!AssetManager::IsHandleValid(audioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioHandle);
			if (!audioSource)
				continue;

			audioSource->Play();
		}

		s_Data.AudioData.PausedAudioSources.clear();
	}

	void AudioSystem::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_Data.ActiveScene, "Invalid scene!");
		VX_CORE_ASSERT(s_Data.ActiveScene->IsRunning(), "Scene must be running!");

		for (const auto& [entityUUID, audioHandle] : s_Data.AudioData.AudioSources)
		{
			Entity entity = s_Data.ActiveScene->TryGetEntityWithUUID(entityUUID);

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
		return s_Data.AudioAPI;
	}

	SharedReference<AudioContext> AudioSystem::GetAudioContext()
	{
		return s_Data.Context;
	}

	void AudioSystem::OnGuiRender()
	{
		Gui::Begin("Audio System");
		Gui::Text("Active Audio Sources: %u", s_Data.AudioData.AudioSources.size());
		for (auto& [entityID, assetHandle] : s_Data.AudioData.AudioSources)
		{
			auto audioSource = Project::GetEditorAssetManager()->GetAsset(assetHandle).As<AudioSource>();
			std::string btnName = fmt::format("Play - {}", audioSource->GetAudioClip().Name);
			if (Gui::Button(btnName.c_str()))
			{
				audioSource->Play();
			}
		}
		Gui::End();
	}

}
