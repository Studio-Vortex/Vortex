#include "vxpch.h"
#include "AudioSystem.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

namespace Vortex {

	struct AudioSystemInternalData
	{
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

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	void AudioSystem::OnContextSceneCreated(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
	}

	void AudioSystem::OnContextSceneDestroyed(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");

		auto view = context->GetAllActorsWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, context };

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

	void AudioSystem::CreateAsset(Actor& actor)
	{

	}

	void AudioSystem::DestroyAsset(Actor& actor)
	{
	}

	void AudioSystem::OnRuntimeStart(Scene* context)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(context, "Invalid scene!");

		auto view = context->GetAllActorsWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, context };
			if (!actor.IsActive())
				continue;

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
			if (!AssetManager::IsHandleValid(asc.AudioHandle))
				continue;

			SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
			if (!audioSource)
				continue;

			// First we have to check if the audio source
			// was playing while editing the scene, and if so, stop it
			PlaybackDevice device = audioSource->GetPlaybackDevice();
			if (device.GetSound().IsPlaying())
			{
				device.Stop();
			}

			if (!asc.PlayOnStart)
				continue;

			device.Play();
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

		auto view = context->GetAllActorsWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, context };
			if (!actor.IsActive())
				continue;

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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

		for (AssetHandle assetHandle : s_Data.PausedAudioSources)
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

		auto view = context->GetAllActorsWith<AudioSourceComponent>();

		for (const auto e : view)
		{
			Actor actor{ e, context };
			if (!actor.IsActive())
				continue;

			const AudioSourceComponent& asc = actor.GetComponent<AudioSourceComponent>();
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
