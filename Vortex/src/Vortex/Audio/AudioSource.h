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
		AudioSource(const std::filesystem::path& path = "");
		~AudioSource();

		const std::filesystem::path& GetPath() const { return m_Path; }
		void SetPath(const std::filesystem::path& path);

		inline PlaybackDevice GetPlaybackDevice() { return m_Device; }

		ASSET_CLASS_TYPE(AudioAsset)

		static SharedReference<AudioSource> Create(const std::filesystem::path& path = "");

	private:
		PlaybackDevice m_Device;
		std::filesystem::path m_Path; // TODO this shouldn't be here
	};

}
