#include "vxpch.h"
#include "Material.h"

namespace Vortex {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

	Material::Material(const SharedRef<Shader>& shader, const MaterialProperties& props)
		: m_Shader(shader), m_Properties(props) { }

	void Material::Bind() const
	{
		if (SharedRef<Texture2D> normalMap = GetNormalMap())
		{
			uint32_t normalMapTextureSlot = 5;
			normalMap->Bind(normalMapTextureSlot);
			m_Shader->SetInt("u_Material.NormalMap", normalMapTextureSlot);
			m_Shader->SetBool("u_Material.HasNormalMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasNormalMap", false);

		if (SharedRef<Texture2D> albedoMap = GetAlbedoMap())
		{
			uint32_t albedoMapTextureSlot = 6;
			albedoMap->Bind(albedoMapTextureSlot);
			m_Shader->SetInt("u_Material.AlbedoMap", albedoMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAlbedoMap", true);
		}
		else
		{
			m_Shader->SetFloat3("u_Material.Albedo", GetAlbedo());
			m_Shader->SetBool("u_Material.HasAlbedoMap", false);
		}

		if (SharedRef<Texture2D> metallicMap = GetMetallicMap())
		{
			uint32_t metallicMapTextureSlot = 7;
			metallicMap->Bind(metallicMapTextureSlot);
			m_Shader->SetInt("u_Material.MetallicMap", metallicMapTextureSlot);
			m_Shader->SetBool("u_Material.HasMetallicMap", true);
		}
		else
		{
			m_Shader->SetFloat("u_Material.Metallic", GetMetallic());
			m_Shader->SetBool("u_Material.HasMetallicMap", false);
		}

		if (SharedRef<Texture2D> roughnessMap = GetRoughnessMap())
		{
			uint32_t roughnessMapTextureSlot = 8;
			roughnessMap->Bind(roughnessMapTextureSlot);
			m_Shader->SetInt("u_Material.RoughnessMap", roughnessMapTextureSlot);
			m_Shader->SetBool("u_Material.HasRoughnessMap", true);
		}
		else
		{
			m_Shader->SetFloat("u_Material.Roughness", GetRoughness());
			m_Shader->SetBool("u_Material.HasRoughnessMap", false);
		}

		if (SharedRef<Texture2D> emissionMap = GetEmissionMap())
		{
			uint32_t emissionMapTextureSlot = 9;
			emissionMap->Bind(emissionMapTextureSlot);
			m_Shader->SetInt("u_Material.EmissionMap", emissionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasEmissionMap", true);
		}
		else
		{
			m_Shader->SetFloat3("u_Material.Emission", GetEmission());
			m_Shader->SetBool("u_Material.HasEmissionMap", false);
		}

		if (SharedRef<Texture2D> parallaxOcclusionMap = GetParallaxOcclusionMap())
		{
			uint32_t parallaxOcclusionMapTextureSlot = 10;
			parallaxOcclusionMap->Bind(parallaxOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.POMap", parallaxOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasPOMap", true);
			m_Shader->SetFloat("u_Material.ParallaxHeightScale", GetParallaxHeightScale());
		}
		else
			m_Shader->SetBool("u_Material.HasPOMap", false);

		if (SharedRef<Texture2D> ambientOcclusionMap = GetAmbientOcclusionMap())
		{
			uint32_t ambientOcclusionMapTextureSlot = 11;
			ambientOcclusionMap->Bind(ambientOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.AOMap", ambientOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAOMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasAOMap", false);
	}

	void Material::Unbind() const { }

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

	Math::vec3 Material::GetEmission() const
	{
		return m_Properties.Emission;
	}

	void Material::SetEmission(const Math::vec3& emission)
	{
		m_Properties.Emission = emission;
	}

	const SharedRef<Texture2D>& Material::GetEmissionMap() const
	{
		return m_Properties.EmissionMap;
	}

	void Material::SetEmissionMap(const SharedRef<Texture2D>& emissionMap)
	{
		m_Properties.EmissionMap = emissionMap;
	}

	float Material::GetParallaxHeightScale() const
	{
		return m_Properties.ParallaxHeightScale;
	}

	void Material::SetParallaxHeightScale(float heightScale)
	{
		m_Properties.ParallaxHeightScale = heightScale;
	}

	const SharedRef<Texture2D>& Material::GetParallaxOcclusionMap() const
	{
		return m_Properties.ParallaxOcclusionMap;
	}

	void Material::SetParallaxOcclusionMap(const SharedRef<Texture2D>& parallaxOcclusionMap)
	{
		m_Properties.ParallaxOcclusionMap = parallaxOcclusionMap;
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
		dstMaterial->SetEmission(srcMaterial->GetEmission());
		dstMaterial->SetEmissionMap(srcMaterial->GetEmissionMap());
		dstMaterial->SetParallaxHeightScale(srcMaterial->GetParallaxHeightScale());
		dstMaterial->SetParallaxOcclusionMap(srcMaterial->GetParallaxOcclusionMap());
		dstMaterial->SetAmbientOcclusionMap(srcMaterial->GetAmbientOcclusionMap());
	}

	SharedRef<Material> Material::Create(const MaterialProperties& props)
	{
		return CreateShared<Material>(props);
	}

	SharedRef<Material> Material::Create(const SharedRef<Shader>& shader, const MaterialProperties& props)
	{
		return CreateShared<Material>(shader, props);
	}

	MaterialInstance::MaterialInstance()
	{
		m_BaseMaterial = MaterialInstance::Create(Material::Create(MaterialProperties()));
	}

	MaterialInstance::MaterialInstance(const SharedRef<Material>& material)
		: m_BaseMaterial(material) { }

	SharedRef<MaterialInstance> MaterialInstance::Create()
	{
		return CreateShared<MaterialInstance>();
	}

	SharedRef<MaterialInstance> MaterialInstance::Create(const SharedRef<Material>& material)
	{
		return CreateShared<MaterialInstance>(material);
	}

}
