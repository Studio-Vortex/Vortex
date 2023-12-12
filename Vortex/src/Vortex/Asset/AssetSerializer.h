#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Asset/Asset.h"
#include "Vortex/Asset/AssetMetadata.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class VORTEX_API AssetSerializer
	{
	public:
		virtual void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) = 0;
		virtual bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) = 0;
	};

	class VORTEX_API MeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API FontSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API AudioSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;

		void SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

	class VORTEX_API SceneAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API PrefabAssetSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API ScriptSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API TextureSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API ParticleEmitterSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;

		void SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

	class VORTEX_API MaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;

		void SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

	class VORTEX_API AnimatorSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API AnimationSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API StaticMeshSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API EnvironmentSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;
	};

	class VORTEX_API AudioListenerSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset) override;

		void SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

	class VORTEX_API PhysicsMaterialSerializer : public AssetSerializer
	{
	public:
		void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset);
	};

}
