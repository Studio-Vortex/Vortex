#include "vxpch.h"
#include "Material.h"

namespace Vortex {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

	void Material::Bind() const
	{
		
	}

	void Material::Unbind() const
	{
		
	}

	const SharedRef<Texture2D>& Material::GetNormalMap() const
	{
		return m_Properties.NormalMap;
	}

	void Material::SetNormalMap(const SharedRef<Texture2D>& normalMap)
	{
		m_Properties.NormalMap = normalMap;
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

	void Material::Copy(const SharedRef<Material>& dstMaterial, const SharedRef<Material>& srcMaterial)
	{
		dstMaterial->SetNormalMap(srcMaterial->GetNormalMap());
		dstMaterial->SetAlbedoMap(srcMaterial->GetAlbedoMap());
		dstMaterial->SetAlbedo(srcMaterial->GetAlbedo());
		dstMaterial->SetMetallicMap(srcMaterial->GetMetallicMap());
		dstMaterial->SetMetallic(srcMaterial->GetMetallic());
		dstMaterial->SetRoughnessMap(srcMaterial->GetRoughnessMap());
		dstMaterial->SetRoughness(srcMaterial->GetRoughness());
		dstMaterial->SetAmbientOcclusionMap(srcMaterial->GetAmbientOcclusionMap());
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
