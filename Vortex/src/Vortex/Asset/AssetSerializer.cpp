#include "vxpch.h"
#include "AssetSerializer.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Renderer/Texture.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	void MeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool MeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void FontSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool FontSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void AudioSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AudioSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void PrefabAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PrefabAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void ScriptSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool ScriptSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path fullyQualifiedPath = Project::GetAssetDirectory() / metadata.Filepath;
		TextureProperties imageProps;
		imageProps.Filepath = fullyQualifiedPath.string();
		imageProps.WrapMode = ImageWrap::Repeat;

		asset = Texture2D::Create(imageProps);
		asset->Handle = metadata.Handle;

		return asset.As<Texture2D>()->IsLoaded();
	}

	void MaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool MaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void AnimatorSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimatorSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void AnimationSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimationSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void EnvironmentSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool EnvironmentSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

	void PhysicsMaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PhysicsMaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return false;
	}

}
