#include "vxpch.h"
#include "AssetImporter.h"

namespace Vortex {

	void AssetImporter::Init()
	{
		s_Serializers[AssetType::Model] = CreateUnique<ModelSerializer>();
		s_Serializers[AssetType::Font] = CreateUnique<FontSerializer>();
		s_Serializers[AssetType::Audio] = CreateUnique<AudioSerializer>();
		s_Serializers[AssetType::Script] = CreateUnique<ScriptSerializer>();
		s_Serializers[AssetType::Texture] = CreateUnique<TextureSerializer>();
		s_Serializers[AssetType::Material] = CreateUnique<MaterialSerializer>();
		s_Serializers[AssetType::Animator] = CreateUnique<AnimatorSerializer>();
		s_Serializers[AssetType::Animation] = CreateUnique<AnimationSerializer>();
		s_Serializers[AssetType::Environment] = CreateUnique<EnvironmentSerializer>();
	}

	void AssetImporter::Shutdown()
	{
		s_Serializers.clear();
	}

	void AssetImporter::Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset)
	{
		s_Serializers[metadata.Type]->Serialize(metadata, asset);
	}

	void AssetImporter::Serialize(const SharedRef<Asset>& asset)
	{
		
	}

	bool AssetImporter::TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset)
	{
		return s_Serializers[metadata.Type]->TryLoadData(metadata, asset);
	}

}
