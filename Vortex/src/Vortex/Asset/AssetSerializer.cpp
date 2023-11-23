#include "vxpch.h"
#include "AssetSerializer.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Audio/AudioSource.h"

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
		SerializeToYAML(metadata, asset);
	}

	bool AudioSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return DeserializeFromYAML(metadata, asset);
	}

	void AudioSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		YAML::Emitter out;

		SharedReference<AudioSource> audioSource = asset.Is<AudioSource>();
		if (!audioSource)
		{
			VX_CONSOLE_LOG_ERROR("Attempting to serialize invalid audio source asset!");
			return;
		}

		const std::string path = audioSource->GetPath().string();
		const std::string trackName = FileSystem::RemoveFileExtension(path);

		Wave::PlaybackDevice device = audioSource->GetPlaybackDevice();

		out << YAML::BeginMap;

		out << YAML::Key << "AudioSource" << YAML::Value << trackName;
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
		{
			VX_SERIALIZE_PROPERTY(AssetHandle, audioSource->Handle, out);
			const std::string filepath = "Audio/" + FileSystem::Relative(audioSource->GetPath(), FileSystem::GetParentDirectory(audioSource->GetPath())).string();
			VX_SERIALIZE_PROPERTY(Filepath, filepath, out);

			out << YAML::Key << "DeviceProperties" << YAML::Value;
			out << YAML::BeginMap; // SoundSettings
			
			const Wave::Vec3 pos = device.GetSound().GetPosition();
			const Math::vec3 position = { pos.X, pos.Y, pos.Z };
			VX_SERIALIZE_PROPERTY(Position, position, out);

			const Wave::Vec3 dir = device.GetSound().GetDirection();
			const Math::vec3 direction = { dir.X, dir.Y, dir.Z };
			VX_SERIALIZE_PROPERTY(Direction, direction, out);
			
			const Wave::Vec3 vel = device.GetSound().GetVelocity();
			const Math::vec3 velocity = { vel.X, vel.Y, vel.Z };
			VX_SERIALIZE_PROPERTY(Velocity, velocity, out);

			out << YAML::Key << "Cone" << YAML::Value;
			out << YAML::BeginMap; // Cone
			const Wave::AudioCone cone = device.GetSound().GetAudioCone();
			VX_SERIALIZE_PROPERTY(InnerAngle, cone.InnerAngle, out);
			VX_SERIALIZE_PROPERTY(OuterAngle, cone.OuterAngle, out);
			VX_SERIALIZE_PROPERTY(OuterGain, cone.OuterGain, out);
			out << YAML::EndMap; // Cone

			const float minGain = device.GetSound().GetMinGain();
			VX_SERIALIZE_PROPERTY(MinGain, minGain, out);

			const float maxGain = device.GetSound().GetMaxGain();
			VX_SERIALIZE_PROPERTY(MaxGain, maxGain, out);

			const AttenuationModel model = (AttenuationModel)device.GetSound().GetAttenuationModel();
			VX_SERIALIZE_PROPERTY(AttenuationModel, Utils::AttenuationModelTypeToString(model), out);

			const float falloff = device.GetSound().GetFalloff();
			VX_SERIALIZE_PROPERTY(Falloff, falloff, out);

			const float minDistance = device.GetSound().GetMinDistance();
			VX_SERIALIZE_PROPERTY(MinDistance, minDistance, out);

			const float maxDistance = device.GetSound().GetMaxDistance();
			VX_SERIALIZE_PROPERTY(MaxDistance, maxDistance, out);

			const float pitch = device.GetSound().GetPitch();
			VX_SERIALIZE_PROPERTY(Pitch, pitch, out);

			const float dopplerFactor = device.GetSound().GetDopplerFactor();
			VX_SERIALIZE_PROPERTY(DopplerFactor, dopplerFactor, out);

			const float volume = device.GetSound().GetVolume();
			VX_SERIALIZE_PROPERTY(Volume, volume, out);

			// TODO: come back when Wave has PlayOneShot and potentially PlayOnStart
			//VX_SERIALIZE_PROPERTY(PlayOnStart, deviceProperties.PlayOnStart, out);
			//VX_SERIALIZE_PROPERTY(PlayOneShot, deviceProperties.PlayOneShot, out);
			
			const bool spacialized = device.GetSound().IsSpacialized();
			VX_SERIALIZE_PROPERTY(Spacialized, spacialized, out);

			const bool looping = device.GetSound().IsLooping();
			VX_SERIALIZE_PROPERTY(Loop, looping, out);
			out << YAML::EndMap; // SoundSettings
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");

		fout << out.c_str();

		fout.close();
	}

	bool AudioSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		YAML::Node audioData = YAML::LoadFile(relativePath);
		if (!audioData)
			return false;

		std::string trackName = audioData["AudioSource"].as<std::string>();

		auto properties = audioData["Properties"];

		std::string filepath = properties["Filepath"].as<std::string>();

		auto deviceProps = properties["DeviceProperties"];

		if (!deviceProps)
			return false;

		std::string fullPath = (Project::GetAssetDirectory() / filepath).string();
		asset = AudioSource::Create(fullPath);
		asset->Handle = metadata.Handle;

		SharedReference<AudioSource> audioSource = asset.Is<AudioSource>();

		Wave::PlaybackDevice device = audioSource->GetPlaybackDevice();
		Wave::Sound sound = device.GetSound();

		const Math::vec3 position = deviceProps["Position"].as<Math::vec3>();
		sound.SetPosition(Wave::Vec3(position.x, position.y, position.z));

		const Math::vec3 direction = deviceProps["Direction"].as<Math::vec3>();
		sound.SetDirection(Wave::Vec3(direction.x, direction.y, direction.z));

		const Math::vec3 velocity = deviceProps["Velocity"].as<Math::vec3>();
		sound.SetVelocity(Wave::Vec3(velocity.x, velocity.y, velocity.z));

		auto coneData = deviceProps["Cone"];
		Wave::AudioCone cone;
		cone.InnerAngle = coneData["InnerAngle"].as<float>();
		cone.OuterAngle = coneData["OuterAngle"].as<float>();
		cone.OuterGain = coneData["OuterGain"].as<float>();
		sound.SetAudioCone(cone);

		float minGain = deviceProps["MinGain"].as<float>();
		sound.SetMinGain(minGain);
		float maxGain = deviceProps["MaxGain"].as<float>();
		sound.SetMaxGain(maxGain);

		std::string modelStr = deviceProps["AttenuationModel"].as<std::string>();
		Wave::AttenuationModel model = (Wave::AttenuationModel)Utils::AttenuationModelTypeFromString(modelStr);
		sound.SetAttenuationModel(model);

		float falloff = deviceProps["Falloff"].as<float>();
		sound.SetFalloff(falloff);

		float minDistance = deviceProps["MinDistance"].as<float>();
		sound.SetMinDistance(minDistance);
		float maxDistance = deviceProps["MaxDistance"].as<float>();
		sound.SetMaxDistance(maxDistance);

		float pitch = deviceProps["Pitch"].as<float>();
		sound.SetPitch(pitch);
		float dopplerFactor = deviceProps["DopplerFactor"].as<float>();
		sound.SetDopplerFactor(dopplerFactor);
		float volume = deviceProps["Volume"].as<float>();
		sound.SetVolume(volume);

		// TODO: ditto
		//data.PlayOnStart = deviceProps["PlayOnStart"].as<bool>();
		//data.PlayOneShot = deviceProps["PlayOneShot"].as<bool>();
		bool spacialized = deviceProps["Spacialized"].as<bool>();
		sound.SetSpacialized(spacialized);
		bool isLooping = deviceProps["Loop"].as<bool>();
		sound.SetLooping(isLooping);

		return true;
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

		auto emitterProps = emitterData["Properties"];

		VX_DESERIALIZE_PROPERTY(ColorBegin, Math::vec4, emitterProperties.ColorBegin, emitterProps);
		VX_DESERIALIZE_PROPERTY(ColorEnd, Math::vec4, emitterProperties.ColorEnd, emitterProps);
		VX_DESERIALIZE_PROPERTY(LifeTime, float, emitterProperties.LifeTime, emitterProps);
		VX_DESERIALIZE_PROPERTY(Position, Math::vec3, emitterProperties.Position, emitterProps);
		VX_DESERIALIZE_PROPERTY(Offset, Math::vec3, emitterProperties.Offset, emitterProps);
		VX_DESERIALIZE_PROPERTY(Rotation, float, emitterProperties.Rotation, emitterProps);
		VX_DESERIALIZE_PROPERTY(SizeBegin, Math::vec2, emitterProperties.SizeBegin, emitterProps);
		VX_DESERIALIZE_PROPERTY(SizeEnd, Math::vec2, emitterProperties.SizeEnd, emitterProps);
		VX_DESERIALIZE_PROPERTY(SizeVariation, Math::vec2, emitterProperties.SizeVariation, emitterProps);
		VX_DESERIALIZE_PROPERTY(Velocity, Math::vec3, emitterProperties.Velocity, emitterProps);
		VX_DESERIALIZE_PROPERTY(VelocityVariation, Math::vec3, emitterProperties.VelocityVariation, emitterProps);
		VX_DESERIALIZE_PROPERTY(GenerateRandomColors, bool, emitterProperties.GenerateRandomColors, emitterProps);

		return true;
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

	void AudioListenerSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		SerializeToYAML(metadata, asset);
	}

	bool AudioListenerSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return DeserializeFromYAML(metadata, asset);
	}

	void AudioListenerSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
	}

	bool AudioListenerSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
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
