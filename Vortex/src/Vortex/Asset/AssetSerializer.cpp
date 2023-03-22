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
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		asset = Mesh::Create(relatvePath.string(), TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<Mesh>()->IsLoaded();
	}

	void FontSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool FontSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		asset = Font::Create(relatvePath);
		asset->Handle = metadata.Handle;

		return asset.As<Font>()->GetFontAtlas()->IsLoaded();
	}

	void AudioSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AudioSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void SceneAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool SceneAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void PrefabAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PrefabAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void ScriptSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool ScriptSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		TextureProperties imageProps;
		imageProps.Filepath = relatvePath.string();
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
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		asset = Material::Create(Renderer::GetShaderLibrary().Get("PBR_Static"), MaterialProperties());
		asset->Handle = metadata.Handle;

		std::string materialName = FileSystem::RemoveFileExtension(relatvePath.filename());
		asset.As<Material>()->SetName(materialName);

		return (bool)asset.Is<Material>();
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
			out << YAML::Key << "RoguhnessMap" << YAML::Value << material->GetRoughnessMap();
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

		std::string outputFile = (Project::GetAssetDirectory() / metadata.Filepath).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file from path!");
		
		fout << out.c_str();

		fout.close();
	}

	bool MaterialSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{

		return false;
	}

	void AnimatorSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimatorSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void AnimationSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimationSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

	void StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		asset = StaticMesh::Create(relatvePath.string(), TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<StaticMesh>()->IsLoaded();
	}

	void EnvironmentSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool EnvironmentSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		asset = Skybox::Create(relatvePath);
		asset->Handle = metadata.Handle;

		return asset.As<Skybox>()->IsLoaded();
	}

	void PhysicsMaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PhysicsMaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::filesystem::path relatvePath = Project::GetAssetDirectory() / metadata.Filepath;

		return false;
	}

}
