#include "sppch.h"
#include "Material.h"

namespace Sparky {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

    const SharedRef<Texture2D>& Material::GetDiffuseMap() const
    {
		return m_Properties.DiffuseMap;
    }

    void Material::SetDiffuseMap(const SharedRef<Texture2D>& diffuseMap)
    {
		m_Properties.DiffuseMap = diffuseMap;
    }

    const SharedRef<Texture2D>& Material::GetSpecularMap() const
    {
		return m_Properties.SpecularMap;
    }

    void Material::SetSpecularMap(const SharedRef<Texture2D>& specularMap)
    {
		m_Properties.SpecularMap = specularMap;
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
