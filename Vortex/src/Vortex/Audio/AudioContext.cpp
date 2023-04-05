#include "vxpch.h"
#include "AudioContext.h"

#include "Vortex/Audio/AudioSystem.h"

#include "Vortex/Platform/MiniAudio/MiniAudioContext.h"

namespace Vortex {

	SharedReference<AudioContext> AudioContext::Create()
	{
		switch (AudioSystem::GetAudioAPI())
		{
			case AudioSystem::AudioAPI::None:      VX_CORE_ASSERT(false, "AudioAPI was set to AudioAPI::None!");
			case AudioSystem::AudioAPI::MiniAudio: return SharedReference<MiniAudioContext>::Create();
		}

		VX_CORE_ASSERT(false, "Unknown audio api");
		return nullptr;
	}

}
