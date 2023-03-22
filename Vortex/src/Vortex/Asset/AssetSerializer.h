#pragma once

#include "Vortex/Asset/Asset.h"
#include "Vortex/Asset/AssetMetadata.h"

#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Utils/FileSystem.h"

namespace YAML {
	
	class Emitter;
	class Node;

}

namespace Vortex {

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) = 0;
		virtual bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) = 0;
	};

	class MeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class AudioSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class SceneAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class PrefabAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class ScriptSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class MaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;

		void SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

	class AnimatorSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class AnimationSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class StaticMeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class PhysicsMaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

}
