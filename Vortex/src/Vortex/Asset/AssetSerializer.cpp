#include "vxpch.h"
#include "AssetSerializer.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Font/Font.h"

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

		size_t length = metadata.Filepath.string().length();
		VX_CORE_ASSERT(length > 0, "Attempting to serialize material asset with invalid filepath!");

		SharedReference<Material> material = asset.Is<Material>();
		if (!material)
		{
			VX_CONSOLE_LOG_ERROR("Attempting to serialize invalid material asset!");
			return;
		}

		std::string materialName = material->GetName();
		if (materialName.empty())
		{
			VX_CONSOLE_LOG_ERROR("Failed to serialize material with no name!");
			return;
		}

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value << materialName;
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
		{
			out << YAML::Key << "AssetHandle" << YAML::Value << material->Handle;
			out << YAML::Key << "Albedo" << YAML::Value << material->GetAlbedo();
			out << YAML::Key << "AlbedoMap" << YAML::Value << material->GetAlbedoMap();
			out << YAML::Key << "NormalMap" << YAML::Value << material->GetNormalMap();
			out << YAML::Key << "Metallic" << YAML::Value << material->GetMetallic();
			out << YAML::Key << "MetallicMap" << YAML::Value << material->GetMetallicMap();
			out << YAML::Key << "Roughness" << YAML::Value << material->GetRoughness();
			out << YAML::Key << "RoughnessMap" << YAML::Value << material->GetRoughnessMap();
			out << YAML::Key << "Emission" << YAML::Value << material->GetEmission();
			out << YAML::Key << "EmissionMap" << YAML::Value << material->GetEmissionMap();
			out << YAML::Key << "ParallaxHeightScale" << YAML::Value << material->GetParallaxHeightScale();
			out << YAML::Key << "ParallaxOcclusionMap" << YAML::Value << material->GetParallaxOcclusionMap();
			out << YAML::Key << "AmbientOcclusionMap" << YAML::Value << material->GetAmbientOcclusionMap();
			out << YAML::Key << "UV" << YAML::Value << material->GetUV();
			out << YAML::Key << "Opacity" << YAML::Value << material->GetOpacity();
			out << YAML::Key << "Flags" << YAML::Value << material->GetFlags();
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file from path!");
		
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
