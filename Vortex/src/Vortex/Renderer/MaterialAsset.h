#pragma once

#include "Vortex/Asset/Asset.h"

namespace Vortex {

	class MaterialAsset : public Asset
	{
	public:
		MaterialAsset();
		~MaterialAsset() override = default;



		static AssetType GetStaticType() { return AssetType::Material; }
		AssetType GetAssetType() const override { return AssetType::Material; }
	private:

	};

}
