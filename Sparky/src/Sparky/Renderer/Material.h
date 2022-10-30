#pragma once

#include "Sparky/Core/Base.h"

namespace Sparky {

	struct SPARKY_API MaterialProperties
	{
		Math::vec3 Ambient = Math::vec3(0.1f, 0.1f, 0.1f);
		Math::vec3 Diffuse = Math::vec3(0.5f, 0.5f, 0.5f);
		Math::vec3 Specular = Math::vec3(0.5f, 0.5f, 0.5f);
		float Shininess = 32.0f;
	};

	class SPARKY_API Material
	{
	public:
		Material(const MaterialProperties& props);

		const Math::vec3& GetAmbient() const;
		void SetAmbient(const Math::vec3& ambient);

		const Math::vec3& GetDiffuse() const;
		void SetDiffuse(const Math::vec3& diffuse);

		const Math::vec3& GetSpecular() const;
		void SetSpecular(const Math::vec3& specular);

		float GetShininess() const;
		void SetShininess(float shininess);

		static SharedRef<Material> Create(const MaterialProperties& props);

	private:
		MaterialProperties m_Properties;
	};
}
