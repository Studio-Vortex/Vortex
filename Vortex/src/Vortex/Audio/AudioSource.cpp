#include "vxpch.h"
#include "AudioSource.h"

#include "Vortex/Audio/Audio.h"

namespace Vortex {

	AudioSource::AudioSource(const std::filesystem::path& path)
	{
		if (path.empty())
			return;

		SetPath(path);
	}

	AudioSource::~AudioSource()
	{
		m_Device.Stop();
		m_Device.Shutdown(Audio::GetContext());
	}

	void AudioSource::SetPath(const std::filesystem::path& path)
	{
		m_Device.Init(Audio::GetContext(), path);
		m_Path = path;
	}

	SharedReference<AudioSource> AudioSource::Create(const std::filesystem::path& path)
	{
		return SharedReference<AudioSource>::Create(path);
	}

}
