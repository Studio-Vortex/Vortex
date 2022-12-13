#include "vxpch.h"
#include "AudioListener.h"

#include "Vortex/Audio/AudioEngine.h"

namespace Vortex {

	AudioListener::AudioListener()
	{
		s_ListenerCount++;
		m_ListenerIndex = s_ListenerCount;
	}

	AudioListener::~AudioListener()
	{
		s_ListenerCount--;
	}

	void AudioListener::SetPosition(const Math::vec3& position)
	{
		
	}

	void AudioListener::SetDirection(const Math::vec3& direction)
	{
	}

	void AudioListener::SetVelocity(const Math::vec3& velocity)
	{
	}

	void AudioListener::SetCone(const ListenerProperties::AudioCone& cone)
	{
	}



}
