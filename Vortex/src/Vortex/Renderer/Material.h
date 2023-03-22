#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Texture.h"

#include <unordered_map>

namespace Vortex {

	enum class VORTEX_API MaterialFlag
	{
		None = 0,
		NoDepthTest = BIT(1),
	};

	struct VORTEX_API MaterialProperties
	{
		std::string Name = "Unnamed";
		Math::vec3 Albedo = Math::vec3(1.0f);
		AssetHandle AlbedoMap = 0;

		AssetHandle NormalMap = 0;

		float Metallic = 0.5f;
		AssetHandle MetallicMap = 0;

		float Roughness = 0.5f;
		AssetHandle RoughnessMap = 0;

		float Emission = 0.0f;
		AssetHandle EmissionMap = 0;

		float ParallaxHeightScale = 0.5f;
		AssetHandle ParallaxOcclusionMap = 0;

		AssetHandle AmbientOcclusionMap = 0;

		Math::vec2 UV = Math::vec2(1.0f);

		float Opacity = 1.0f;

		uint32_t Flags = 0;
	};

	class VORTEX_API Material : public Asset
	{
	public:
		Material() = default;
		Material(SharedReference<Shader>& shader, const MaterialProperties& props);
		~Material() override = default;

		void Bind() const;
		void Unbind() const;

		const SharedReference<Shader>& GetShader() const;
		void SetShader(SharedReference<Shader>& shader);

		const std::string& GetName() const;
		void SetName(const std::string& name);

		const Math::vec3& GetAlbedo() const;
		void SetAlbedo(const Math::vec3& albedo);

		AssetHandle GetAlbedoMap() const;
		void SetAlbedoMap(AssetHandle albedoMap);

		AssetHandle GetNormalMap() const;
		void SetNormalMap(AssetHandle normalMap);

		float GetMetallic() const;
		void SetMetallic(float metallic);

		AssetHandle GetMetallicMap() const;
		void SetMetallicMap(AssetHandle metallicMap);

		float GetRoughness() const;
		void SetRoughness(float roughness);

		AssetHandle GetRoughnessMap() const;
		void SetRoughnessMap(AssetHandle roughnessMap);

		float GetEmission() const;
		void SetEmission(float emission);

		AssetHandle GetEmissionMap() const;
		void SetEmissionMap(AssetHandle emissionMap);

		float GetParallaxHeightScale() const;
		void SetParallaxHeightScale(float heightScale);

		AssetHandle GetParallaxOcclusionMap() const;
		void SetParallaxOcclusionMap(AssetHandle parallaxOcclusionMap);

		AssetHandle GetAmbientOcclusionMap() const;
		void SetAmbientOcclusionMap(AssetHandle ambientOcclusionMap);

		const Math::vec2& GetUV() const;
		void SetUV(const Math::vec2& uv);

		float GetOpacity() const;
		void SetOpacity(float opacity);

		// Flags
		uint32_t GetFlags() const;
		bool HasFlag(MaterialFlag flag) const;
		void SetFlag(MaterialFlag flag);
		void SetFlags(MaterialFlag* flags, uint32_t count);
		void SetFlags(uint32_t flags);
		void ToggleFlag(MaterialFlag flag);
		void ToggleFlags(MaterialFlag* flags, uint32_t count);
		void RemoveFlag(MaterialFlag flag);
		void RemoveFlags(MaterialFlag* flags, uint32_t count);
		void ClearFlags();

		static void Copy(SharedReference<Material>& dest, const SharedReference<Material>& src);

		ASSET_CLASS_TYPE(MaterialAsset)

		static SharedReference<Material> Create(SharedReference<Shader>& shader, const MaterialProperties& props);

	protected:
		MaterialProperties m_Properties;
		SharedReference<Shader> m_Shader = nullptr;
	};

	class MaterialTable : public RefCounted
	{
	public:
		MaterialTable() = default;
		~MaterialTable() = default;

		AssetHandle GetMaterial(uint32_t submeshIndex) const;
		void SetMaterial(uint32_t submeshIndex, AssetHandle materialHandle);

	private:
		std::unordered_map<uint32_t, AssetHandle> m_Materials;
	};

}
