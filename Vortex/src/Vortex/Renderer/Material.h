#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Texture.h"

namespace Vortex {

	struct VORTEX_API MaterialProperties
	{
		SharedRef<Texture2D> NormalMap = nullptr;

		Math::vec3 Albedo = Math::vec3(1.0f);
		SharedRef<Texture2D> AlbedoMap = nullptr;

		float Metallic = 0.5f;
		SharedRef<Texture2D> MetallicMap = nullptr;

		float Roughness = 0.5f;
		SharedRef<Texture2D> RoughnessMap = nullptr;

		SharedRef<Texture2D> AmbientOcclusionMap = nullptr;
	};

	class VORTEX_API Material
	{
	public:
		Material() = default;
		Material(const MaterialProperties& props);

		void Bind() const;
		void Unbind() const;

		const SharedRef<Texture2D>& GetNormalMap() const;
		void SetNormalMap(const SharedRef<Texture2D>& normalMap);

		const Math::vec3& GetAlbedo() const;
		void SetAlbedo(const Math::vec3& albedo);

		const SharedRef<Texture2D>& GetAlbedoMap() const;
		void SetAlbedoMap(const SharedRef<Texture2D>& albedoMap);

		float GetMetallic() const;
		void SetMetallic(float metallic);

		const SharedRef<Texture2D>& GetMetallicMap() const;
		void SetMetallicMap(const SharedRef<Texture2D>& metallicMap);

		float GetRoughness() const;
		void SetRoughness(float roughness);

		const SharedRef<Texture2D>& GetRoughnessMap() const;
		void SetRoughnessMap(const SharedRef<Texture2D>& roughnessMap);

		const SharedRef<Texture2D>& GetAmbientOcclusionMap() const;
		void SetAmbientOcclusionMap(const SharedRef<Texture2D>& ambientOcclusionMap);

		static void Copy(const SharedRef<Material>& dstMaterial, const SharedRef<Material>& srcMaterial);

		static SharedRef<Material> Create(const MaterialProperties& props);

	protected:
		MaterialProperties m_Properties;
	};

	class VORTEX_API MaterialInstance : public Material
	{
	public:
		MaterialInstance();
		MaterialInstance(const SharedRef<Material>& material);
		~MaterialInstance() = default;

		static SharedRef<MaterialInstance> Create();
		static SharedRef<MaterialInstance> Create(const SharedRef<Material>& material);

	private:
		SharedRef<Material> m_BaseMaterial;
	};

}
