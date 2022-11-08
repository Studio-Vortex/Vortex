#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Texture.h"

namespace Sparky {

	struct SPARKY_API MaterialProperties
	{
		Math::vec3 Ambient = Math::vec3(1.0f);
		SharedRef<Texture2D> DiffuseMap = nullptr;
		SharedRef<Texture2D> SpecularMap = nullptr;
		SharedRef<Texture2D> NormalMap = nullptr;
		float Shininess = 32.0f;

		SharedRef<Texture2D> AlbedoMap = nullptr;
		SharedRef<Texture2D> MetallicMap = nullptr;
		SharedRef<Texture2D> RoughnessMap = nullptr;
		SharedRef<Texture2D> AmbientOcclusionMap = nullptr;
	};

	class SPARKY_API Material
	{
	public:
		Material(const MaterialProperties& props);

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

		const SharedRef<Texture2D>& GetAlbedoMap() const;
		void SetAlbedoMap(const SharedRef<Texture2D>& albedoMap);

		const SharedRef<Texture2D>& GetMetallicMap() const;
		void SetMetallicMap(const SharedRef<Texture2D>& metallicMap);

		const SharedRef<Texture2D>& GetRoughnessMap() const;
		void SetRoughnessMap(const SharedRef<Texture2D>& roughnessMap);

		const SharedRef<Texture2D>& GetAmbientOcclusionMap() const;
		void SetAmbientOcclusionMap(const SharedRef<Texture2D>& ambientOcclusionMap);

		static SharedRef<Material> Create(const MaterialProperties& props);

	private:
		MaterialProperties m_Properties;
	};
}
