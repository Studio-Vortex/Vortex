#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Texture.h"

namespace Sparky {

	struct SPARKY_API MaterialProperties
	{
		SharedRef<Shader> Shader;

		Math::vec3 Ambient = Math::vec3(1.0f);
		SharedRef<Texture2D> DiffuseMap = nullptr;
		SharedRef<Texture2D> SpecularMap = nullptr;
		SharedRef<Texture2D> NormalMap = nullptr;
		float Shininess = 32.0f;

		Math::vec3 Albedo = Math::vec3(1.0f);
		SharedRef<Texture2D> AlbedoMap = nullptr;

		float Metallic = 0.5f;
		SharedRef<Texture2D> MetallicMap = nullptr;

		float Roughness = 0.5f;
		SharedRef<Texture2D> RoughnessMap = nullptr;

		SharedRef<Texture2D> AmbientOcclusionMap = nullptr;
	};

	class SPARKY_API Material
	{
	public:
		Material() = default;
		Material(const MaterialProperties& props);

		void Bind() const;
		void Unbind() const;

		const SharedRef<Shader>& GetShader() const;

		const Math::vec3& GetAmbient() const;
		void SetAmbient(const Math::vec3& ambient);

		const SharedRef<Texture2D>& GetDiffuseMap() const;
		void SetDiffuseMap(const SharedRef<Texture2D>& diffuseMap);

		const SharedRef<Texture2D>& GetSpecularMap() const;
		void SetSpecularMap(const SharedRef<Texture2D>& specularMap);

		const SharedRef<Texture2D>& GetNormalMap() const;
		void SetNormalMap(const SharedRef<Texture2D>& normalMap);

		float GetShininess() const;
		void SetShininess(float shininess);

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

		static SharedRef<Material> Create(const MaterialProperties& props);

	protected:
		MaterialProperties m_Properties;
	};

	class SPARKY_API MaterialInstance : public Material
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
