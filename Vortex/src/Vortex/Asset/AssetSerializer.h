#pragma once

#include "Vortex/Asset/AssetMetadata.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) = 0;
		virtual bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) = 0;
	};

	class MeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class StaticMeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class AudioSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class ScriptSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class TextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class MaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class AnimatorSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class AnimationSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class SubTextureSerializer : public AssetSerializer
	{
	public:
		// TODO these shouldn't be here but are required right now
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override {}
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override { return false; }

		// TODO these should override the derived types functions in the future once all assets
		// are managed by the asset manager
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset> asset);
	};

	class EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

	class PhysicsMaterialSerializer : public AssetSerializer
	{
	public:
		// ditto
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override {}
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override { return false;}

		// ditto
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset> asset);
	};

}
