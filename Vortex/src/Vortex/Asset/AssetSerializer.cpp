#include "vxpch.h"
#include "AssetSerializer.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/Utils/YAML_SerializationUtils.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	void MeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool MeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = Mesh::Create(relativePath, TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<Mesh>()->IsLoaded();
	}

	void FontSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool FontSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = Font::Create(relativePath);
		asset->Handle = metadata.Handle;

		return asset.As<Font>()->GetFontAtlas()->IsLoaded();
	}

	void AudioSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AudioSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void PrefabAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PrefabAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void ScriptSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool ScriptSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		TextureProperties imageProps;
		imageProps.Filepath = relativePath;
		imageProps.WrapMode = ImageWrap::Repeat;

		asset = Texture2D::Create(imageProps);
		asset->Handle = metadata.Handle;

		return asset.As<Texture2D>()->IsLoaded();
	}

	void ParticleEmitterSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		SerializeToYAML(metadata, asset);
	}

	bool ParticleEmitterSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		asset = ParticleEmitter::Create({});
		asset->Handle = metadata.Handle;

		return DeserializeFromYAML(metadata, asset);
	}

	void ParticleEmitterSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		YAML::Emitter out;

		SharedReference<ParticleEmitter> particleEmitter = asset.Is<ParticleEmitter>();
		if (!particleEmitter)
		{
			VX_CONSOLE_LOG_ERROR("Attempting to serialize invalid particle emitter asset!");
			return;
		}

		const std::string& particleEmitterName = particleEmitter->GetName();
		const ParticleEmitterProperties emitterProperties = particleEmitter->GetProperties();

		out << YAML::BeginMap;

		out << YAML::Key << "ParticleEmitter" << YAML::Value << particleEmitterName;
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
		{
			VX_SERIALIZE_PROPERTY(AssetHandle, particleEmitter->Handle, out);
			VX_SERIALIZE_PROPERTY(ColorBegin, emitterProperties.ColorBegin, out);
			VX_SERIALIZE_PROPERTY(ColorEnd, emitterProperties.ColorEnd, out);
			VX_SERIALIZE_PROPERTY(LifeTime, emitterProperties.LifeTime, out);
			VX_SERIALIZE_PROPERTY(Position, emitterProperties.Position, out);
			VX_SERIALIZE_PROPERTY(Offset, emitterProperties.Offset, out);
			VX_SERIALIZE_PROPERTY(Rotation, emitterProperties.Rotation, out);
			VX_SERIALIZE_PROPERTY(SizeBegin, emitterProperties.SizeBegin, out);
			VX_SERIALIZE_PROPERTY(SizeEnd, emitterProperties.SizeEnd, out);
			VX_SERIALIZE_PROPERTY(SizeVariation, emitterProperties.SizeVariation, out);
			VX_SERIALIZE_PROPERTY(Velocity, emitterProperties.Velocity, out);
			VX_SERIALIZE_PROPERTY(VelocityVariation, emitterProperties.VelocityVariation, out);
			VX_SERIALIZE_PROPERTY(GenerateRandomColors, emitterProperties.GenerateRandomColors, out);
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");

		fout << out.c_str();

		fout.close();
	}

	bool ParticleEmitterSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		YAML::Node emitterData = YAML::LoadFile(relativePath);
		if (!emitterData)
			return false;

		std::string emitterName = emitterData["ParticleEmitter"].as<std::string>();

		SharedReference<ParticleEmitter> particleEmitter = asset.Is<ParticleEmitter>();
		particleEmitter->SetName(emitterName);

		ParticleEmitterProperties& emitterProperties = particleEmitter->GetProperties();

		VX_DESERIALIZE_PROPERTY(ColorBegin, Math::vec4, emitterProperties.ColorBegin, emitterData);
		VX_DESERIALIZE_PROPERTY(ColorEnd, Math::vec4, emitterProperties.ColorEnd, emitterData);
		VX_DESERIALIZE_PROPERTY(LifeTime, float, emitterProperties.LifeTime, emitterData);
		VX_DESERIALIZE_PROPERTY(Position, Math::vec3, emitterProperties.Position, emitterData);
		VX_DESERIALIZE_PROPERTY(Offset, Math::vec3, emitterProperties.Offset, emitterData);
		VX_DESERIALIZE_PROPERTY(Rotation, float, emitterProperties.Rotation, emitterData);
		VX_DESERIALIZE_PROPERTY(SizeBegin, Math::vec2, emitterProperties.SizeBegin, emitterData);
		VX_DESERIALIZE_PROPERTY(SizeEnd, Math::vec2, emitterProperties.SizeEnd, emitterData);
		VX_DESERIALIZE_PROPERTY(SizeVariation, Math::vec2, emitterProperties.SizeVariation, emitterData);
		VX_DESERIALIZE_PROPERTY(Velocity, Math::vec3, emitterProperties.Velocity, emitterData);
		VX_DESERIALIZE_PROPERTY(VelocityVariation, Math::vec3, emitterProperties.VelocityVariation, emitterData);
		VX_DESERIALIZE_PROPERTY(GenerateRandomColors, bool, emitterProperties.GenerateRandomColors, emitterData);

		return false;
	}

	void MaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		SerializeToYAML(metadata, asset);
	}

	bool MaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		asset = Material::Create(Renderer::GetShaderLibrary().Get("PBR_Static"), MaterialProperties());
		asset->Handle = metadata.Handle;

		return DeserializeFromYAML(metadata, asset);
	}

	void MaterialSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		YAML::Emitter out;

		SharedReference<Material> material = asset.Is<Material>();
		if (!material)
		{
			VX_CONSOLE_LOG_ERROR("Attempting to serialize invalid material asset!");
			return;
		}

		const std::string& materialName = material->GetName();
		if (materialName.empty())
		{
			VX_CONSOLE_LOG_ERROR("Failed to serialize material with no name!");
			return;
		}

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value << materialName;
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
		{
			VX_SERIALIZE_PROPERTY(AssetHandle, material->Handle, out);
			VX_SERIALIZE_PROPERTY(Albedo, material->GetAlbedo(), out);
			VX_SERIALIZE_PROPERTY(AlbedoMap, material->GetAlbedoMap(), out);
			VX_SERIALIZE_PROPERTY(NormalMap, material->GetNormalMap(), out);
			VX_SERIALIZE_PROPERTY(Metallic, material->GetMetallic(), out);
			VX_SERIALIZE_PROPERTY(MetallicMap, material->GetMetallicMap(), out);
			VX_SERIALIZE_PROPERTY(Roughness, material->GetRoughness(), out);
			VX_SERIALIZE_PROPERTY(RoughnessMap, material->GetRoughnessMap(), out);
			VX_SERIALIZE_PROPERTY(Emission, material->GetEmission(), out);
			VX_SERIALIZE_PROPERTY(EmissionMap, material->GetEmissionMap(), out);
			VX_SERIALIZE_PROPERTY(ParallaxHeightScale, material->GetParallaxHeightScale(), out);
			VX_SERIALIZE_PROPERTY(ParallaxOcclusionMap, material->GetParallaxOcclusionMap(), out);
			VX_SERIALIZE_PROPERTY(AmbientOcclusionMap, material->GetAmbientOcclusionMap(), out);
			VX_SERIALIZE_PROPERTY(UV, material->GetUV(), out);
			VX_SERIALIZE_PROPERTY(Opacity, material->GetOpacity(), out);
			VX_SERIALIZE_PROPERTY(Flags, material->GetFlags(), out);
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");
		
		fout << out.c_str();

		fout.close();
	}

	bool MaterialSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		
		YAML::Node materialData = YAML::LoadFile(relativePath);
		if (!materialData)
			return false;

		std::string materialName = materialData["Material"].as<std::string>();

		auto materialProperties = materialData["Properties"];
		if (!materialProperties)
			return false;

		SharedReference<Material> material = asset.As<Material>();

		material->SetName(materialName);
		material->SetAlbedo(materialProperties["Albedo"].as<Math::vec3>());
		material->SetAlbedoMap(materialProperties["AlbedoMap"].as<uint64_t>());
		material->SetNormalMap(materialProperties["NormalMap"].as<uint64_t>());
		material->SetMetallic(materialProperties["Metallic"].as<float>());
		material->SetMetallicMap(materialProperties["MetallicMap"].as<uint64_t>());
		material->SetRoughness(materialProperties["Roughness"].as<float>());
		material->SetRoughnessMap(materialProperties["RoughnessMap"].as<uint64_t>());
		material->SetEmission(materialProperties["Emission"].as<float>());
		material->SetEmissionMap(materialProperties["EmissionMap"].as<uint64_t>());
		material->SetParallaxHeightScale(materialProperties["ParallaxHeightScale"].as<float>());
		material->SetParallaxOcclusionMap(materialProperties["ParallaxOcclusionMap"].as<uint64_t>());
		material->SetAmbientOcclusionMap(materialProperties["AmbientOcclusionMap"].as<uint64_t>());
		material->SetUV(materialProperties["UV"].as<Math::vec2>());
		material->SetOpacity(materialProperties["Opacity"].as<float>());
		material->SetFlags(materialProperties["Flags"].as<uint32_t>());

		return true;
	}

	void AnimatorSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimatorSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void AnimationSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimationSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = StaticMesh::Create(relativePath, TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<StaticMesh>()->IsLoaded();
	}

	void EnvironmentSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool EnvironmentSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = Skybox::Create(relativePath);
		asset->Handle = metadata.Handle;

		return asset.As<Skybox>()->IsLoaded();
	}

	void PhysicsMaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PhysicsMaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

}
