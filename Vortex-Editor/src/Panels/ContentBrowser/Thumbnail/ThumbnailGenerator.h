#pragma once

#include <Vortex.h>

#include "AssetThumbnailGenerator.h"

#include <unordered_map>

namespace Vortex {

	class ThumbnailGenerator : public RefCounted
	{
	public:
		void Init();
		void Shutdown();

		SharedReference<Texture2D> GenerateThumbnail(SharedReference<Asset> asset);

	private:
		std::unordered_map<AssetType, UniqueRef<AssetThumbnailGenerator>> m_Generators;
	};

}
