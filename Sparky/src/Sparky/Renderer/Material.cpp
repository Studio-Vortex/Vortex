#include "sppch.h"
#include "Material.h"

namespace Sparky {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

	void Material::Bind() const
	{
		if (m_Properties.AlbedoMap)
		{
			m_Properties.AlbedoMap->Bind(0);
			m_Properties.Shader->SetInt("u_Material.AlbedoMap", 0);
		}
		if (m_Properties.AmbientOcclusionMap)
		{
			m_Properties.AmbientOcclusionMap->Bind(1);
			m_Properties.Shader->SetInt("u_Material.AOMap", 1);
		}
		if (m_Properties.DiffuseMap)
		{
			m_Properties.DiffuseMap->Bind(2);
			m_Properties.Shader->SetInt("u_Material.DiffuseMap", 2);
		}
		if (m_Properties.MetallicMap)
		{
			m_Properties.MetallicMap->Bind(3);
			m_Properties.Shader->SetInt("u_Material.MetallicMap", 3);
		}
		if (m_Properties.NormalMap)
		{
			m_Properties.NormalMap->Bind(4);
			m_Properties.Shader->SetInt("u_Material.NormalMap", 4);
		}
		if (m_Properties.RoughnessMap)
		{
			m_Properties.RoughnessMap->Bind(5);
			m_Properties.Shader->SetInt("u_Material.RoughnessMap", 5);
		}
		if (m_Properties.SpecularMap)
		{
			m_Properties.SpecularMap->Bind(6);
			m_Properties.Shader->SetInt("u_Material.SpecularMap", 6);
		}
	}

	void Material::Unbind() const
	{
		if (m_Properties.AlbedoMap)
			m_Properties.AlbedoMap->Unbind();
		if (m_Properties.AmbientOcclusionMap)
			m_Properties.AmbientOcclusionMap->Unbind();
		if (m_Properties.DiffuseMap)
			m_Properties.DiffuseMap->Unbind();
		if (m_Properties.MetallicMap)
			m_Properties.MetallicMap->Unbind();
		if (m_Properties.NormalMap)
			m_Properties.NormalMap->Unbind();
		if (m_Properties.RoughnessMap)
			m_Properties.RoughnessMap->Unbind();
		if (m_Properties.SpecularMap)
			m_Properties.SpecularMap->Unbind();
	}

    const SharedRef<Shader>& Material::GetShader() const
    {
		return m_Properties.Shader;
    }

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

    const Math::vec3& Material::GetAlbedo() const
    {
		return m_Properties.Albedo;
    }

    void Material::SetAlbedo(const Math::vec3& albedo)
    {
		m_Properties.Albedo = albedo;
    }

    const SharedRef<Texture2D>& Material::GetAlbedoMap() const
    {
		return m_Properties.AlbedoMap;
    }

    void Material::SetAlbedoMap(const SharedRef<Texture2D>& albedoMap)
    {
		m_Properties.AlbedoMap = albedoMap;
    }

    float Material::GetMetallic() const
    {
		return m_Properties.Metallic;
    }

    void Material::SetMetallic(float metallic)
    {
		m_Properties.Metallic = metallic;
    }

    const SharedRef<Texture2D>& Material::GetMetallicMap() const
    {
		return m_Properties.MetallicMap;
    }

    void Material::SetMetallicMap(const SharedRef<Texture2D>& metallicMap)
    {
		m_Properties.MetallicMap = metallicMap;
    }

    float Material::GetRoughness() const
    {
		return m_Properties.Roughness;
    }

    void Material::SetRoughness(float roughness)
    {
		m_Properties.Roughness = roughness;
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

	MaterialInstance::MaterialInstance()
	{
		m_BaseMaterial = MaterialInstance::Create(Material::Create(MaterialProperties()));
	}

	MaterialInstance::MaterialInstance(const SharedRef<Material>& material)
		: m_BaseMaterial(material)
	{

	}

	SharedRef<MaterialInstance> MaterialInstance::Create()
	{
		return CreateShared<MaterialInstance>();
	}

	SharedRef<MaterialInstance> MaterialInstance::Create(const SharedRef<Material>& material)
	{
		return CreateShared<MaterialInstance>(material);
	}

}
