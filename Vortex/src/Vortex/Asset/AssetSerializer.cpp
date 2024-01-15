#include "vxpch.h"
#include "AssetSerializer.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Prefab.h"

#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Audio/AudioTypes.h"
#include "Vortex/Audio/AudioUtils.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/Serialization/SceneSerializer.h"

#include "Vortex/Utils/YAML_SerializationUtils.h"
#include "Vortex/Utils/FileSystem.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	void MeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool MeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = Mesh::Create(relativePath, TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<Mesh>()->IsLoaded();
	}

	void FontSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool FontSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

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
			VX_CONSOLE_LOG_ERROR("[Asset Serializer] Attempting to serialize invalid audio source asset!");
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
			
			const Math::vec3 position = Utils::FromWaveVector(device.GetSound().GetPosition());
			VX_SERIALIZE_PROPERTY(Position, position, out);

			const Math::vec3 direction = Utils::FromWaveVector(device.GetSound().GetDirection());
			VX_SERIALIZE_PROPERTY(Direction, direction, out);
			
			const Math::vec3 velocity = Utils::FromWaveVector(device.GetSound().GetVelocity());
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

			const AttenuationModel attModel = Utils::FromWaveAttenuationModel(device.GetSound().GetAttenuationModel());
			VX_SERIALIZE_PROPERTY(AttenuationModel, Utils::AttenuationModelTypeToString(attModel), out);

			const float pan = device.GetSound().GetPan();
			VX_SERIALIZE_PROPERTY(Pan, pan, out);

			const PanMode panMode = Utils::FromWavePanMode(device.GetSound().GetPanMode());
			VX_SERIALIZE_PROPERTY(PanMode, Utils::PanModeTypeToString(panMode), out);

			const PositioningMode positioningMode = Utils::FromWavePositioningMode(device.GetSound().GetPositioning());
			VX_SERIALIZE_PROPERTY(PositioningMode, Utils::PositioningModeTypeToString(positioningMode), out);

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

			const float directionalAttenuationFactor = device.GetSound().GetDirectionalAttenuationFactor();
			VX_SERIALIZE_PROPERTY(DirectionalAttenuationFactor, directionalAttenuationFactor, out);

			const float volume = device.GetSound().GetVolume();
			VX_SERIALIZE_PROPERTY(Volume, volume, out);
			
			const bool spacialized = device.GetSound().IsSpacialized();
			VX_SERIALIZE_PROPERTY(Spacialized, spacialized, out);

			const bool looping = device.GetSound().IsLooping();
			VX_SERIALIZE_PROPERTY(Loop, looping, out);
			out << YAML::EndMap; // SoundSettings
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		const std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");

		fout << out.c_str();

		fout.close();
	}

	bool AudioSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		YAML::Node audioData = YAML::LoadFile(relativePath);
		if (!audioData)
			return false;

		const std::string trackName = audioData["AudioSource"].as<std::string>();

		auto properties = audioData["Properties"];

		const std::string filepath = properties["Filepath"].as<std::string>();

		auto deviceProps = properties["DeviceProperties"];

		if (!deviceProps)
			return false;

		const std::string fullPath = (Project::GetAssetDirectory() / filepath).string();
		asset = AudioSource::Create(fullPath);
		asset->Handle = metadata.Handle;

		SharedReference<AudioSource> audioSource = asset.Is<AudioSource>();

		Wave::PlaybackDevice device = audioSource->GetPlaybackDevice();
		Wave::Sound sound = device.GetSound();

		const Math::vec3 position = deviceProps["Position"].as<Math::vec3>();
		sound.SetPosition(Utils::ToWaveVector(position));

		const Math::vec3 direction = deviceProps["Direction"].as<Math::vec3>();
		sound.SetDirection(Utils::ToWaveVector(direction));

		const Math::vec3 velocity = deviceProps["Velocity"].as<Math::vec3>();
		sound.SetVelocity(Utils::ToWaveVector(velocity));

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

		const std::string attModelStr = deviceProps["AttenuationModel"].as<std::string>();
		sound.SetAttenuationModel(Utils::ToWaveAttenuationModel(Utils::AttenuationModelTypeFromString(attModelStr)));

		float pan = deviceProps["Pan"].as<float>();
		sound.SetPan(pan);

		const std::string panModeStr = deviceProps["PanMode"].as<std::string>();
		sound.SetPanMode(Utils::ToWavePanMode(Utils::PanModeTypeFromString(panModeStr)));

		const std::string positioningModeStr = deviceProps["PositioningMode"].as<std::string>();
		sound.SetPositioning(Utils::ToWavePositioningMode(Utils::PositioningModeTypeFromString(positioningModeStr)));

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
		float directionalAttenuationFactor = deviceProps["DirectionalAttenuationFactor"].as<float>();
		sound.SetDirectionalAttenuationFactor(directionalAttenuationFactor);
		float volume = deviceProps["Volume"].as<float>();
		sound.SetVolume(volume);

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
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void PrefabAssetSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		SerializeToYAML(metadata, asset);
	}

	bool PrefabAssetSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		return DeserializeFromYAML(metadata, asset);
	}

	void PrefabAssetSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		YAML::Emitter out;

		SharedReference<Prefab> prefab = asset.Is<Prefab>();
		if (!prefab)
		{
			VX_CONSOLE_LOG_ERROR("[Asset Serializer] Attempting to serialize invalid prefab asset!");
			return;
		}

		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		prefab->m_Scene->m_Registry.each([&](auto actorID) {
			Actor actor = { actorID, prefab->m_Scene.Raw() };
			if (!actor || !actor.HasComponent<IDComponent>())
				return;

			SceneSerializer::SerializeActor(out, actor);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		const std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");

		fout << out.c_str();

		fout.close();
	}

	bool PrefabAssetSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = SharedReference<Prefab>::Create();
		asset->Handle = metadata.Handle;

		YAML::Node prefabData = YAML::LoadFile(relativePath);
		if (!prefabData)
			return false;

		const YAML::Node prefabNode = prefabData["Prefab"];

		SceneSerializer::DeserializeActors(prefabNode, asset.As<Prefab>()->m_Scene);

		return true;
	}

	void ScriptSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool ScriptSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void TextureSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool TextureSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

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
		return DeserializeFromYAML(metadata, asset);
	}

	void ParticleEmitterSerializer::SerializeToYAML(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{
		YAML::Emitter out;

		SharedReference<ParticleEmitter> particleEmitter = asset.Is<ParticleEmitter>();
		if (!particleEmitter)
		{
			VX_CONSOLE_LOG_ERROR("[Asset Serializer] Attempting to serialize invalid particle emitter asset!");
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

		const std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");

		fout << out.c_str();

		fout.close();
	}

	bool ParticleEmitterSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		YAML::Node emitterData = YAML::LoadFile(relativePath);
		if (!emitterData)
			return false;

		const std::string emitterName = emitterData["ParticleEmitter"].as<std::string>();

		auto properties = emitterData["Properties"];

		if (!properties)
			return false;

		asset = ParticleEmitter::Create({});
		asset->Handle = metadata.Handle;

		SharedReference<ParticleEmitter> particleEmitter = asset.Is<ParticleEmitter>();
		particleEmitter->SetName(emitterName);

		ParticleEmitterProperties emitterProperties;

		VX_DESERIALIZE_PROPERTY(ColorBegin, Math::vec4, emitterProperties.ColorBegin, properties);
		VX_DESERIALIZE_PROPERTY(ColorEnd, Math::vec4, emitterProperties.ColorEnd, properties);
		VX_DESERIALIZE_PROPERTY(LifeTime, float, emitterProperties.LifeTime, properties);
		VX_DESERIALIZE_PROPERTY(Position, Math::vec3, emitterProperties.Position, properties);
		VX_DESERIALIZE_PROPERTY(Offset, Math::vec3, emitterProperties.Offset, properties);
		VX_DESERIALIZE_PROPERTY(Rotation, float, emitterProperties.Rotation, properties);
		VX_DESERIALIZE_PROPERTY(SizeBegin, Math::vec2, emitterProperties.SizeBegin, properties);
		VX_DESERIALIZE_PROPERTY(SizeEnd, Math::vec2, emitterProperties.SizeEnd, properties);
		VX_DESERIALIZE_PROPERTY(SizeVariation, Math::vec2, emitterProperties.SizeVariation, properties);
		VX_DESERIALIZE_PROPERTY(Velocity, Math::vec3, emitterProperties.Velocity, properties);
		VX_DESERIALIZE_PROPERTY(VelocityVariation, Math::vec3, emitterProperties.VelocityVariation, properties);
		VX_DESERIALIZE_PROPERTY(GenerateRandomColors, bool, emitterProperties.GenerateRandomColors, properties);

		particleEmitter->SetProperties(emitterProperties);

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
			VX_CONSOLE_LOG_ERROR("[Asset Serializer] Attempting to serialize invalid material asset!");
			return;
		}

		const std::string& materialName = material->GetName();
		if (materialName.empty())
		{
			VX_CONSOLE_LOG_ERROR("[Asset Serializer] Attempting to serialize material with no name!");
			return;
		}

		out << YAML::BeginMap;

		out << YAML::Key << "Material" << YAML::Value << materialName;
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginMap;
		{
			VX_SERIALIZE_PROPERTY(AssetHandle, material->Handle, out);
			VX_SERIALIZE_PROPERTY(Albedo, material->GetAlbedo(), out);
			VX_SERIALIZE_PROPERTY(AlbedoMap, material->GetTexture("u_AlbedoMap"), out);
			VX_SERIALIZE_PROPERTY(NormalMap, material->GetTexture("u_NormalMap"), out);
			VX_SERIALIZE_PROPERTY(Metallic, material->GetMetallic(), out);
			VX_SERIALIZE_PROPERTY(MetallicMap, material->GetTexture("u_MetallicMap"), out);
			VX_SERIALIZE_PROPERTY(Roughness, material->GetRoughness(), out);
			VX_SERIALIZE_PROPERTY(RoughnessMap, material->GetTexture("u_RoughnessMap"), out);
			VX_SERIALIZE_PROPERTY(Emission, material->GetEmission(), out);
			VX_SERIALIZE_PROPERTY(EmissionMap, material->GetTexture("u_EmissionMap"), out);
			VX_SERIALIZE_PROPERTY(ParallaxHeightScale, material->GetParallaxHeightScale(), out);
			VX_SERIALIZE_PROPERTY(ParallaxOcclusionMap, material->GetTexture("u_ParallaxOcclusionMap"), out);
			VX_SERIALIZE_PROPERTY(AmbientOcclusionMap, material->GetTexture("u_AmbientOcclusionMap"), out);
			VX_SERIALIZE_PROPERTY(UV, material->GetUV(), out);
			VX_SERIALIZE_PROPERTY(Opacity, material->GetOpacity(), out);
			VX_SERIALIZE_PROPERTY(Flags, material->GetFlags(), out);
		}
		out << YAML::EndMap;
		out << YAML::EndMap;

		const std::string outputFile = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		std::ofstream fout(outputFile);
		VX_CORE_ASSERT(fout.is_open(), "Failed to open file!");
		
		fout << out.c_str();

		fout.close();
	}

	bool MaterialSerializer::DeserializeFromYAML(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();
		
		YAML::Node materialData = YAML::LoadFile(relativePath);
		if (!materialData)
			return false;

		const std::string materialName = materialData["Material"].as<std::string>();

		auto materialProperties = materialData["Properties"];
		if (!materialProperties)
			return false;

		SharedReference<Material> material = asset.As<Material>();

		material->SetName(materialName);
		material->SetAlbedo(materialProperties["Albedo"].as<Math::vec3>());
		material->SetTexture("u_AlbedoMap", materialProperties["AlbedoMap"].as<uint64_t>());
		material->SetTexture("u_NormalMap", materialProperties["NormalMap"].as<uint64_t>());
		material->SetMetallic(materialProperties["Metallic"].as<float>());
		material->SetTexture("u_MetallicMap", materialProperties["MetallicMap"].as<uint64_t>());
		material->SetRoughness(materialProperties["Roughness"].as<float>());
		material->SetTexture("u_RoughnessMap", materialProperties["RoughnessMap"].as<uint64_t>());
		material->SetEmission(materialProperties["Emission"].as<float>());
		material->SetTexture("u_EmissionMap", materialProperties["EmissionMap"].as<uint64_t>());
		material->SetParallaxHeightScale(materialProperties["ParallaxHeightScale"].as<float>());
		material->SetTexture("u_ParallaxOcclusionMap", materialProperties["ParallaxOcclusionMap"].as<uint64_t>());
		material->SetTexture("u_AmbientOcclusionMap", materialProperties["AmbientOcclusionMap"].as<uint64_t>());
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
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void AnimationSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool AnimationSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void StaticMeshSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool StaticMeshSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		asset = StaticMesh::Create(relativePath, TransformComponent(), MeshImportOptions());
		asset->Handle = metadata.Handle;

		return asset.As<StaticMesh>()->IsLoaded();
	}

	void EnvironmentSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool EnvironmentSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

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
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

	void PhysicsMaterialSerializer::Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset)
	{

	}

	bool PhysicsMaterialSerializer::TryLoadData(const AssetMetadata& metadata, SharedReference<Asset>& asset)
	{
		const std::string relativePath = Project::GetEditorAssetManager()->GetFileSystemPath(metadata).string();

		return false;
	}

}
