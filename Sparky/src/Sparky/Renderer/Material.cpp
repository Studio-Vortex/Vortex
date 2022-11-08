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

    const SharedRef<Texture2D>& Material::GetNormalMap() const
    {
		return m_Properties.NormalMap;
    }

    void Material::SetNormalMap(const SharedRef<Texture2D>& normalMap)
    {
		m_Properties.NormalMap = normalMap;
    }

	float Material::GetShininess() const
	{
		return m_Properties.Shininess;
	}

	void Material::SetShininess(float shininess)
	{
		m_Properties.Shininess = shininess;
	}

    const SharedRef<Texture2D>& Material::GetAlbedoMap() const
    {
		return m_Properties.AlbedoMap;
    }

    void Material::SetAlbedoMap(const SharedRef<Texture2D>& albedoMap)
    {
		m_Properties.AlbedoMap = albedoMap;
    }

    const SharedRef<Texture2D>& Material::GetMetallicMap() const
    {
		return m_Properties.MetallicMap;
    }

    void Material::SetMetallicMap(const SharedRef<Texture2D>& metallicMap)
    {
		m_Properties.MetallicMap = metallicMap;
    }

    const SharedRef<Texture2D>& Material::GetRoughnessMap() const
    {
		return m_Properties.RoughnessMap;
    }

    void Material::SetRoughnessMap(const SharedRef<Texture2D>& roughnessMap)
    {
		m_Properties.RoughnessMap = roughnessMap;
    }

    const SharedRef<Texture2D>& Material::GetAmbientOcclusionMap() const
    {
		return m_Properties.AmbientOcclusionMap;
    }

    void Material::SetAmbientOcclusionMap(const SharedRef<Texture2D>& ambientOcclusionMap)
    {
		m_Properties.AmbientOcclusionMap = ambientOcclusionMap;
    }

	SharedRef<Material> Material::Create(const MaterialProperties& props)
	{
		return CreateShared<Material>(props);
	}

}
