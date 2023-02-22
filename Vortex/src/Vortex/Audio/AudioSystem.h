#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioListener.h"

#include <unordered_map>

namespace Vortex {

	class VORTEX_API AudioSystem
	{
	public:
		static void Init();
		static void Shutdown();

		

	private:
		inline static std::unordered_map<UUID, SharedRef<AudioSource>> s_ActiveAudioSources;
		inline static std::unordered_map<UUID, SharedRef<AudioListener>> s_ActiveAudioListeners;
	};

}
