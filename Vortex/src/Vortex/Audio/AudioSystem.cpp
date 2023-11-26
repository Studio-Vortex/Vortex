#include "vxpch.h"
#include "AudioSystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

namespace Vortex {

	struct AudioSystemInternalData
	{
		using AudioSourceData = std::unordered_map<UUID, AssetHandle>;
		AudioSourceData SourceData;
		std::vector<AssetHandle> PausedAudioSources;

		SubModule Module;
	};

	static AudioSystemInternalData s_Data;

	AudioSystem::AudioSystem()
		: IAssetSystem("Audio System")
	{
	}

	void AudioSystem::Init()
	{
		Audio::Init();

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Audio";
		moduleProps.APIVersion = Version(1, 3, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void AudioSystem::Shutdown()
	{
		Audio::Shutdown();

		s_Data.SourceData.clear();

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void AudioSystem::OnContextSceneCreated(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		s_Data.SourceData.clear();
	}

	void AudioSystem::OnContextSceneDestroyed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		s_Data.SourceData.clear();
	}

	void AudioSystem::CreateAsset(Entity& entity)
	{

	}

	void AudioSystem::DestroyAsset(Entity& entity)
	{
	}

	void AudioSystem::OnRuntimeStart(Scene* context)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(context, "Invalid scene!");

		auto view = context->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, context };
			if (!entity.IsActive())
				continue;

			const auto& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				continue;

			// First we check to see if it's already playing
			if (audioSource->GetPlaybackDevice().GetSound().IsPlaying())
			{
				audioSource->GetPlaybackDevice().Stop();
			}

			// TODO once Wave has PlayOnStart or something similar we can handle it here
		}
	}

	void AudioSystem::OnUpdateRuntime(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void AudioSystem::OnRuntimeScenePaused(Scene* context)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(context, "Invalid scene!");

		auto view = context->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, context };
			if (!entity.IsActive())
				continue;

			const auto& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				continue;

#ifdef VX_DEBUG
			if (!audioSource->GetPlaybackDevice().Pause())
			{
				const std::string& err = Audio::GetContext()->GetLastErrorMsg();
				VX_CONSOLE_LOG_ERROR("[Audio] {}", err);
				continue;
			}
#else
			audioSource->GetPlaybackDevice().Pause();
#endif // VX_DEBUG

			s_Data.PausedAudioSources.push_back(asc.AudioHandle);
		}
	}

	void AudioSystem::OnRuntimeSceneResumed(Scene* context)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (const auto& assetHandle : s_Data.PausedAudioSources)
		{
			if (!AssetManager::IsHandleValid(assetHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(assetHandle);
			if (!audioSource)
				continue;

#ifdef VX_DEBUG
			if (!audioSource->GetPlaybackDevice().Play())
			{
				const std::string& err = Audio::GetContext()->GetLastErrorMsg();
				VX_CONSOLE_LOG_ERROR("[Audio] {}", err);
				continue;
			}
#else
			audioSource->GetPlaybackDevice().Play();
#endif // VX_DEBUG

		}

		s_Data.PausedAudioSources.clear();
	}

	void AudioSystem::OnRuntimeStop(Scene* context)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(context, "Invalid scene!");

		auto view = context->GetAllEntitiesWith<AudioSourceComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, context };
			if (!entity.IsActive())
				continue;

			const auto& asc = entity.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				continue;

			if (!audioSource->GetPlaybackDevice().GetSound().IsPlaying())
				continue;

			audioSource->GetPlaybackDevice().Stop();
		}
	}

	void AudioSystem::OnGuiRender()
	{
	}

}
