#pragma once

#include "Vortex/Asset/AssetMetadata.h"
#include "Vortex/Scene/Scene.h"

namespace Vortex {

	class AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) = 0;
		virtual bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) = 0;
	};

	class ModelSerializer : public AssetSerializer
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

	class EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;
	};

}
