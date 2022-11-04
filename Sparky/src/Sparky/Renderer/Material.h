#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Renderer/Texture.h"

namespace Sparky {

	struct SPARKY_API MaterialProperties
	{
		Math::vec3 Ambient = Math::vec3(1.0f);
		SharedRef<Texture2D> DiffuseMap = nullptr;
		SharedRef<Texture2D> SpecularMap = nullptr;
		float Shininess = 32.0f;
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

		float GetShininess() const;
		void SetShininess(float shininess);

		static SharedRef<Material> Create(const MaterialProperties& props);

	private:
		MaterialProperties m_Properties;
	};
}
