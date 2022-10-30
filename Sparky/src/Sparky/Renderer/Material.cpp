#include "sppch.h"
#include "Material.h"

namespace Sparky {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

	const Math::vec3& Material::GetAmbient() const
	{
		return m_Properties.Ambient;
	}

	void Material::SetAmbient(const Math::vec3& ambient)
	{
		m_Properties.Ambient = ambient;
	}

	const Math::vec3& Material::GetDiffuse() const
	{
		return m_Properties.Diffuse;
	}

	void Material::SetDiffuse(const Math::vec3& diffuse)
	{
		m_Properties.Diffuse = diffuse;
	}

	const Math::vec3& Material::GetSpecular() const
	{
		return m_Properties.Specular;
	}

	void Material::SetSpecular(const Math::vec3& specular)
	{
		m_Properties.Specular = specular;
	}

	float Material::GetShininess() const
	{
		return m_Properties.Shininess;
	}

	void Material::SetShininess(float shininess)
	{
		m_Properties.Shininess = shininess;
	}

	SharedRef<Material> Material::Create(const MaterialProperties& props)
	{
		return CreateShared<Material>(props);
	}

}
