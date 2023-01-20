#include "vxpch.h"
#include "Material.h"

namespace Vortex {

	Material::Material(const MaterialProperties& props)
		: m_Properties(props) { }

	Material::Material(const SharedRef<Shader>& shader, const MaterialProperties& props)
		: m_Shader(shader), m_Properties(props) { }

	void Material::Bind() const
	{
		if (SharedRef<Texture2D> normalMap = m_Properties.NormalMap)
		{
			uint32_t normalMapTextureSlot = 5;
			normalMap->Bind(normalMapTextureSlot);
			m_Shader->SetInt("u_Material.NormalMap", normalMapTextureSlot);
			m_Shader->SetBool("u_Material.HasNormalMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasNormalMap", false);

		if (SharedRef<Texture2D> albedoMap = m_Properties.AlbedoMap)
		{
			uint32_t albedoMapTextureSlot = 6;
			albedoMap->Bind(albedoMapTextureSlot);
			m_Shader->SetInt("u_Material.AlbedoMap", albedoMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAlbedoMap", true);
		}
		else
		{
			m_Shader->SetFloat3("u_Material.Albedo", m_Properties.Albedo);
			m_Shader->SetBool("u_Material.HasAlbedoMap", false);
		}

		if (SharedRef<Texture2D> metallicMap = m_Properties.MetallicMap)
		{
			uint32_t metallicMapTextureSlot = 7;
			metallicMap->Bind(metallicMapTextureSlot);
			m_Shader->SetInt("u_Material.MetallicMap", metallicMapTextureSlot);
			m_Shader->SetBool("u_Material.HasMetallicMap", true);
		}
		else
		{
			m_Shader->SetFloat("u_Material.Metallic", m_Properties.Metallic);
			m_Shader->SetBool("u_Material.HasMetallicMap", false);
		}

		if (SharedRef<Texture2D> roughnessMap = m_Properties.RoughnessMap)
		{
			uint32_t roughnessMapTextureSlot = 8;
			roughnessMap->Bind(roughnessMapTextureSlot);
			m_Shader->SetInt("u_Material.RoughnessMap", roughnessMapTextureSlot);
			m_Shader->SetBool("u_Material.HasRoughnessMap", true);
		}
		else
		{
			m_Shader->SetFloat("u_Material.Roughness", m_Properties.Roughness);
			m_Shader->SetBool("u_Material.HasRoughnessMap", false);
		}

		if (SharedRef<Texture2D> emissionMap = m_Properties.EmissionMap)
		{
			uint32_t emissionMapTextureSlot = 9;
			emissionMap->Bind(emissionMapTextureSlot);
			m_Shader->SetInt("u_Material.EmissionMap", emissionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasEmissionMap", true);
		}
		else
		{
			m_Shader->SetFloat3("u_Material.Emission", m_Properties.Emission);
			m_Shader->SetBool("u_Material.HasEmissionMap", false);
		}

		if (SharedRef<Texture2D> parallaxOcclusionMap = m_Properties.ParallaxOcclusionMap)
		{
			uint32_t parallaxOcclusionMapTextureSlot = 10;
			parallaxOcclusionMap->Bind(parallaxOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.POMap", parallaxOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasPOMap", true);
			m_Shader->SetFloat("u_Material.ParallaxHeightScale", m_Properties.ParallaxHeightScale);
		}
		else
			m_Shader->SetBool("u_Material.HasPOMap", false);

		if (SharedRef<Texture2D> ambientOcclusionMap = m_Properties.AmbientOcclusionMap)
		{
			uint32_t ambientOcclusionMapTextureSlot = 11;
			ambientOcclusionMap->Bind(ambientOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.AOMap", ambientOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAOMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasAOMap", false);

		m_Shader->SetFloat("u_Material.Opacity", m_Properties.Opacity);
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

    float Material::GetOpacity() const
    {
        return m_Properties.Opacity;
    }

    void Material::SetOpacity(float opacity)
    {
		m_Properties.Opacity = opacity;
    }

	uint32_t Material::GetFlags() const
	{
		return m_Properties.Flags;
	}

	bool Material::HasFlag(MaterialFlag flag) const
	{
		VX_CORE_ASSERT(flag != MaterialFlag::None, "Unknown Material Flag!");
		return m_Properties.Flags & (uint32_t)flag;
	}

	void Material::SetFlag(MaterialFlag flag)
	{
		VX_CORE_ASSERT(flag != MaterialFlag::None, "Unknown Material Flag!");
		m_Properties.Flags |= (uint32_t)flag;
	}

	void Material::SetFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			VX_CORE_ASSERT(flags[i] != MaterialFlag::None, "Unknown Material Flag!");
			SetFlag(flags[i]);
		}
	}

	void Material::SetFlags(uint32_t flags)
	{
		ClearFlags();
		m_Properties.Flags = flags;
	}

	void Material::ToggleFlag(MaterialFlag flag)
	{
		VX_CORE_ASSERT(flag != MaterialFlag::None, "Unknown Material Flag!");
		m_Properties.Flags ^= (uint32_t)flag;
	}

	void Material::ToggleFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			VX_CORE_ASSERT(flags[i] != MaterialFlag::None, "Unknown Material Flag!");
			ToggleFlag(flags[i]);
		}
	}

	void Material::RemoveFlag(MaterialFlag flag)
	{
		VX_CORE_ASSERT(flag != MaterialFlag::None, "Unknown Material Flag!");
		m_Properties.Flags &= (uint32_t)flag;
	}

	void Material::RemoveFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			VX_CORE_ASSERT(flags[i] != MaterialFlag::None, "Unknown Material Flag!");
			RemoveFlag(flags[i]);
		}
	}

	void Material::ClearFlags()
	{
		memset(&m_Properties.Flags, 0, sizeof(uint32_t));
	}

	void Material::Copy(const SharedRef<Material>& dest, const SharedRef<Material>& src)
	{
		dest->SetNormalMap(src->GetNormalMap());
		dest->SetAlbedoMap(src->GetAlbedoMap());
		dest->SetAlbedo(src->GetAlbedo());
		dest->SetMetallicMap(src->GetMetallicMap());
		dest->SetMetallic(src->GetMetallic());
		dest->SetRoughnessMap(src->GetRoughnessMap());
		dest->SetRoughness(src->GetRoughness());
		dest->SetEmission(src->GetEmission());
		dest->SetEmissionMap(src->GetEmissionMap());
		dest->SetParallaxHeightScale(src->GetParallaxHeightScale());
		dest->SetParallaxOcclusionMap(src->GetParallaxOcclusionMap());
		dest->SetAmbientOcclusionMap(src->GetAmbientOcclusionMap());
		dest->SetOpacity(src->GetOpacity());
		dest->ClearFlags();
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
