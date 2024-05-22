#include "vxpch.h"
#include "Material.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Shader.h"
#include "Vortex/Renderer/Texture.h"

#include "Vortex/Asset/AssetManager.h"

namespace Vortex {

	Material::Material(SharedReference<Shader> shader, const MaterialProperties& props)
		: m_Shader(shader), m_Properties(props) { }

	void Material::Bind() const
	{
		if (AssetHandle handle = GetTexture("u_NormalMap"))
		{
			uint32_t normalMapTextureSlot = 6;
			SharedReference<Texture2D> normalMap = AssetManager::GetAsset<Texture2D>(handle);
			normalMap->Bind(normalMapTextureSlot);
			m_Shader->SetInt("u_Material.NormalMap", normalMapTextureSlot);
			m_Shader->SetBool("u_Material.HasNormalMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasNormalMap", false);

		m_Shader->SetFloat3("u_Material.Albedo", m_Properties.Albedo);
		if (AssetHandle handle = GetTexture("u_AlbedoMap"))
		{
			uint32_t albedoMapTextureSlot = 7;
			SharedReference<Texture2D> albedoMap = AssetManager::GetAsset<Texture2D>(handle);
			albedoMap->Bind(albedoMapTextureSlot);
			m_Shader->SetInt("u_Material.AlbedoMap", albedoMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAlbedoMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasAlbedoMap", false);

		m_Shader->SetFloat("u_Material.Metallic", m_Properties.Metallic);
		if (AssetHandle handle = GetTexture("u_MetallicMap"))
		{
			uint32_t metallicMapTextureSlot = 8;
			SharedReference<Texture2D> metallicMap = AssetManager::GetAsset<Texture2D>(handle);
			metallicMap->Bind(metallicMapTextureSlot);
			m_Shader->SetInt("u_Material.MetallicMap", metallicMapTextureSlot);
			m_Shader->SetBool("u_Material.HasMetallicMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasMetallicMap", false);

		m_Shader->SetFloat("u_Material.Roughness", m_Properties.Roughness);
		if (AssetHandle handle = GetTexture("u_RoughnessMap"))
		{
			uint32_t roughnessMapTextureSlot = 9;
			SharedReference<Texture2D> roughnessMap = AssetManager::GetAsset<Texture2D>(handle);
			roughnessMap->Bind(roughnessMapTextureSlot);
			m_Shader->SetInt("u_Material.RoughnessMap", roughnessMapTextureSlot);
			m_Shader->SetBool("u_Material.HasRoughnessMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasRoughnessMap", false);

		m_Shader->SetFloat("u_Material.Emission", m_Properties.Emission);
		if (AssetHandle handle = GetTexture("u_EmissionMap"))
		{
			uint32_t emissionMapTextureSlot = 10;
			SharedReference<Texture2D> emissionMap = AssetManager::GetAsset<Texture2D>(handle);
			emissionMap->Bind(emissionMapTextureSlot);
			m_Shader->SetInt("u_Material.EmissionMap", emissionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasEmissionMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasEmissionMap", false);

		if (AssetHandle handle = GetTexture("u_ParallaxOcclusionMap"))
		{
			uint32_t parallaxOcclusionMapTextureSlot = 11;
			SharedReference<Texture2D> parallaxOcclusionMap = AssetManager::GetAsset<Texture2D>(handle);
			parallaxOcclusionMap->Bind(parallaxOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.POMap", parallaxOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasPOMap", true);
			m_Shader->SetFloat("u_Material.ParallaxHeightScale", m_Properties.ParallaxHeightScale);
		}
		else
			m_Shader->SetBool("u_Material.HasPOMap", false);

		if (AssetHandle handle = GetTexture("u_AmbientOcclusionMap"))
		{
			uint32_t ambientOcclusionMapTextureSlot = 12;
			SharedReference<Texture2D> ambientOcclusionMap = AssetManager::GetAsset<Texture2D>(handle);
			ambientOcclusionMap->Bind(ambientOcclusionMapTextureSlot);
			m_Shader->SetInt("u_Material.AOMap", ambientOcclusionMapTextureSlot);
			m_Shader->SetBool("u_Material.HasAOMap", true);
		}
		else
			m_Shader->SetBool("u_Material.HasAOMap", false);

		m_Shader->SetFloat("u_Material.Opacity", m_Properties.Opacity);
	}

	void Material::Unbind() const { }

	SharedReference<Shader> Material::GetShader() const
	{
		return m_Shader;
	}

	void Material::SetShader(SharedReference<Shader> shader)
	{
		m_Shader = shader;
	}

    const std::string& Material::GetName() const
    {
		return m_Properties.Name;
    }

    void Material::SetName(const std::string& name)
    {
		m_Properties.Name = name;
    }

    AssetHandle Material::GetTexture(const std::string& name) const
    {
		if (!HasTexture(name)) {
			return 0;
		}

		return m_Properties.Textures.at(name);
    }

    void Material::SetTexture(const std::string& name, AssetHandle texture)
    {
		m_Properties.Textures[name] = texture;
    }

	bool Material::HasTexture(const std::string& name) const
	{
		return m_Properties.Textures.contains(name);
	}

	const Math::vec3& Material::GetAlbedo() const
	{
		return m_Properties.Albedo;
	}

	void Material::SetAlbedo(const Math::vec3& albedo)
	{
		m_Properties.Albedo = albedo;
	}

	float Material::GetMetallic() const
	{
		return m_Properties.Metallic;
	}

	void Material::SetMetallic(float metallic)
	{
		m_Properties.Metallic = metallic;
	}

	float Material::GetRoughness() const
	{
		return m_Properties.Roughness;
	}

	void Material::SetRoughness(float roughness)
	{
		m_Properties.Roughness = roughness;
	}

	float Material::GetEmission() const
	{
		return m_Properties.Emission;
	}

	void Material::SetEmission(float emission)
	{
		m_Properties.Emission = emission;
	}

	float Material::GetParallaxHeightScale() const
	{
		return m_Properties.ParallaxHeightScale;
	}

	void Material::SetParallaxHeightScale(float heightScale)
	{
		m_Properties.ParallaxHeightScale = heightScale;
	}

    const Math::vec2& Material::GetUV() const
    {
		return m_Properties.UV;
    }

    void Material::SetUV(const Math::vec2& uv)
    {
		m_Properties.UV = uv;
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
		return m_Properties.Flags & (uint32_t)flag;
	}

	void Material::SetFlag(MaterialFlag flag)
	{
		m_Properties.Flags |= (uint32_t)flag;
	}

	void Material::SetFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
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
		m_Properties.Flags ^= (uint32_t)flag;
	}

	void Material::ToggleFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			ToggleFlag(flags[i]);
		}
	}

	void Material::RemoveFlag(MaterialFlag flag)
	{
		m_Properties.Flags &= (uint32_t)flag;
	}

	void Material::RemoveFlags(MaterialFlag* flags, uint32_t count)
	{
		for (uint32_t i = 0; i < count; i++)
		{
			RemoveFlag(flags[i]);
		}
	}

	void Material::ClearFlags()
	{
		memset(&m_Properties.Flags, 0, sizeof(uint32_t));
	}

	void Material::Copy(SharedReference<Material>& dest, const SharedReference<Material>& src)
	{
		dest->m_Properties.Textures = src->m_Properties.Textures;
		dest->SetAlbedo(src->GetAlbedo());
		dest->SetMetallic(src->GetMetallic());
		dest->SetRoughness(src->GetRoughness());
		dest->SetEmission(src->GetEmission());
		dest->SetParallaxHeightScale(src->GetParallaxHeightScale());
		dest->SetUV(src->GetUV());
		dest->SetOpacity(src->GetOpacity());
		dest->ClearFlags();
		dest->SetFlags(src->GetFlags());
	}

	AssetHandle Material::GetDefaultMaterialHandle()
	{
		return s_DefaultMaterialHandle;
	}

	SharedReference<Material> Material::Create(const SharedReference<Shader>& shader, const MaterialProperties& props)
	{
		return SharedReference<Material>::Create(shader, props);
	}

	void Material::SetDefaultMaterialHandle(AssetHandle assetHandle)
	{
		s_DefaultMaterialHandle = assetHandle;
	}

    AssetHandle MaterialTable::GetMaterial(uint32_t submeshIndex) const
    {
		if (m_Materials.contains(submeshIndex))
			return m_Materials.at(submeshIndex);

		VX_CORE_ASSERT(false, "Index out of bounds!");
		return 0;
    }

    void MaterialTable::SetMaterial(uint32_t submeshIndex, AssetHandle materialHandle)
    {
		m_Materials[submeshIndex] = materialHandle;
    }

    bool MaterialTable::HasMaterial(uint32_t submeshIndex) const
    {
        return m_Materials.contains(submeshIndex);
    }

	void MaterialTable::Clear()
	{
		m_Materials.clear();
	}

	bool MaterialTable::Empty() const
	{
		return m_Materials.empty();
	}

	uint32_t MaterialTable::GetMaterialCount() const
	{
		return m_Materials.size();
	}

}
