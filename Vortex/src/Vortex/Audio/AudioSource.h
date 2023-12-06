#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Utils/FileSystem.h"

#include <Wave/Wave.h>

namespace Vortex {

	using PlaybackDevice = Wave::PlaybackDevice;

	class VORTEX_API AudioSource : public Asset
	{
	public:
		AudioSource(const Fs::Path& path = "");
		~AudioSource();

		const Fs::Path& GetPath() const { return m_Filepath; }
		void SetPath(const Fs::Path& path);

		inline PlaybackDevice GetPlaybackDevice() { return m_Device; }

		ASSET_CLASS_TYPE(AudioAsset)

		static SharedReference<AudioSource> Create(const Fs::Path& path = "");

	private:
		PlaybackDevice m_Device;
		Fs::Path m_Filepath; // TODO this shouldn't be here
	};

}
