#include "vxpch.h"
#include "AudioSource.h"

#include "Vortex/Audio/Audio.h"

namespace Vortex {

	AudioSource::AudioSource(const Fs::Path& path)
	{
		if (path.empty())
			return;

		SetPath(path);
	}

	AudioSource::~AudioSource()
	{
		// NOTE:
		// Instead of cleaning up an audio source here
		// they all get cleaned up in the scene's destructor.
		// So if an AudioSource is used elsewhere outside a scene
		// it must be cleaned up manually by calling Shutdown() on the playback device
	}

	void AudioSource::SetPath(const Fs::Path& path)
	{
		m_Device.Init(Audio::GetContext(), path);
		m_Filepath = path;
	}

	SharedReference<AudioSource> AudioSource::Create(const Fs::Path& path)
	{
		return SharedReference<AudioSource>::Create(path);
	}

}
