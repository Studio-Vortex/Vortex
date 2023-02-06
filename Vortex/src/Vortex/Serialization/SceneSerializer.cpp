#include "vxpch.h"
#include "SceneSerializer.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Model.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/Font/Font.h"

#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Utils/YAML_SerializationUtils.h"

#include <fstream>

namespace Vortex {

#define WRITE_SCRIPT_FIELD(FieldType, Type)   \
	case ScriptFieldType::FieldType:          \
		out << scriptField.GetValue<Type>();  \
		break;

#define READ_SCRIPT_FIELD(FieldType, Type)            \
	case ScriptFieldType::FieldType:                  \
	{                                                 \
		Type data = scriptField["Data"].as<Type>();   \
		fieldInstance.SetValue(data);                 \
		break;                                        \
	}

	namespace Utils {

		static std::string LightTypeToString(LightType lightType)
		{
			switch (lightType)
			{
				case LightType::Directional:  return "Directional";
				case LightType::Point:        return "Point";
				case LightType::Spot:         return "Spot";
			}

			VX_CORE_ASSERT(false, "Unknown Light Type!");
			return {};
		}

		static LightType LightTypeFromString(const std::string& lightTypeString)
		{
			if (lightTypeString == "Directional")  return LightType::Directional;
			if (lightTypeString == "Point")        return LightType::Point;
			if (lightTypeString == "Spot")         return LightType::Spot;

			VX_CORE_ASSERT(false, "Unknown Light Type!");
			return LightType::Directional;
		}

		static std::string MeshTypeToString(MeshType meshType)
		{
			switch (meshType)
			{
				case MeshType::Cube:     return "Cube";
				case MeshType::Sphere:   return "Sphere";
				case MeshType::Capsule:  return "Capsule";
				case MeshType::Cone:     return "Cone";
				case MeshType::Cylinder: return "Cylinder";
				case MeshType::Plane:    return "Plane";
				case MeshType::Torus:    return "Torus";
				case MeshType::Custom:   return "Custom";
			}

			VX_CORE_ASSERT(false, "Unknown Mesh Type!");
			return {};
		}

		static MeshType MeshTypeFromString(const std::string& meshTypeString)
		{
			if (meshTypeString == "Cube")     return MeshType::Cube;
			if (meshTypeString == "Sphere")   return MeshType::Sphere;
			if (meshTypeString == "Capsule")  return MeshType::Capsule;
			if (meshTypeString == "Cone")     return MeshType::Cone;
			if (meshTypeString == "Cylinder") return MeshType::Cylinder;
			if (meshTypeString == "Plane")    return MeshType::Plane;
			if (meshTypeString == "Torus")    return MeshType::Torus;
			if (meshTypeString == "Custom")   return MeshType::Custom;

			VX_CORE_ASSERT(false, "Unknown Mesh Type!");
			return MeshType::Cube;
		}

		static std::string RigidBody2DBodyTypeToString(RigidBody2DType bodyType)
		{
			switch (bodyType)
			{
				case RigidBody2DType::Static:    return "Static";
				case RigidBody2DType::Dynamic:   return "Dynamic";
				case RigidBody2DType::Kinematic: return "Kinematic";
			}

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return {};
		}

		static RigidBody2DType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
		{
			if (bodyTypeString == "Static")    return RigidBody2DType::Static;
			if (bodyTypeString == "Dynamic")   return RigidBody2DType::Dynamic;
			if (bodyTypeString == "Kinematic") return RigidBody2DType::Kinematic;

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return RigidBody2DType::Static;
		}

		static std::string RigidBodyBodyTypeToString(RigidBodyType bodyType)
		{
			switch (bodyType)
			{
				case RigidBodyType::Static:    return "Static";
				case RigidBodyType::Dynamic:   return "Dynamic";
			}

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return {};
		}

		static RigidBodyType RigidBodyBodyTypeFromString(const std::string& bodyTypeString)
		{
			if (bodyTypeString == "Static")    return RigidBodyType::Static;
			if (bodyTypeString == "Dynamic")   return RigidBodyType::Dynamic;

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return RigidBodyType::Static;
		}

		static std::string CollisionDetectionTypeToString(CollisionDetectionType collisionDetection)
		{
			switch (collisionDetection)
			{
				case Vortex::CollisionDetectionType::Discrete:              return "Discrete";
				case Vortex::CollisionDetectionType::Continuous:            return "Continuous";
				case Vortex::CollisionDetectionType::ContinuousSpeculative: return "ContinuousSpeculative";
			}

			VX_CORE_ASSERT(false, "Unknown Collision Detection Type!");
			return {};
		}

		static CollisionDetectionType CollisionDetectionTypeFromString(const std::string& collisionDetectionString)
		{
			if (collisionDetectionString == "Discrete")              return CollisionDetectionType::Discrete;
			if (collisionDetectionString == "Continuous")            return CollisionDetectionType::Continuous;
			if (collisionDetectionString == "ContinuousSpeculative") return CollisionDetectionType::ContinuousSpeculative;

			VX_CORE_ASSERT(false, "Unknown Collision Detection Type!");
			return CollisionDetectionType::None;
		}

		static std::string NonWalkableModeToString(NonWalkableMode mode)
		{
			switch (mode)
			{
				case Vortex::NonWalkableMode::PreventClimbing:                return "PreventClimbing";
				case Vortex::NonWalkableMode::PreventClimbingAndForceSliding: return "PreventClimbingAndForceSliding";
			}

			VX_CORE_ASSERT(false, "Unknown Non Walkable Mode!");
			return "";
		}

		static NonWalkableMode NonWalkableModeFromString(const std::string& walkableMode)
		{
			if (walkableMode == "PreventClimbing") return NonWalkableMode::PreventClimbing;
			if (walkableMode == "PreventClimbingAndForceSliding") return NonWalkableMode::PreventClimbingAndForceSliding;

			VX_CORE_ASSERT(false, "Unknown Non Walkable Mode!");
			return NonWalkableMode::PreventClimbing;
		}

		static std::string CapsuleClimbModeToString(CapsuleClimbMode mode)
		{
			switch (mode)
			{
				case CapsuleClimbMode::Easy:        return "Easy";
				case CapsuleClimbMode::Constrained: return "Constrained";
			}

			VX_CORE_ASSERT(false, "Unknown Capsule Climb Mode!");
			return "";
		}

		static CapsuleClimbMode CapsuleClimbModeFromString(const std::string& climbMode)
		{
			if (climbMode == "Easy")        return CapsuleClimbMode::Easy;
			if (climbMode == "Constrained") return CapsuleClimbMode::Constrained;

			VX_CORE_ASSERT(false, "Unknown Capsule Climb Mode!");
			return CapsuleClimbMode::Easy;
		}

		static std::string CombineModeToString(CombineMode mode)
		{
			switch (mode)
			{
				case CombineMode::Average:  return "Average";
				case CombineMode::Max:      return "Max";
				case CombineMode::Min:      return "Min";
				case CombineMode::Mulitply: return "Multiply";
			}

			VX_CORE_ASSERT(false, "Unknown Combine Mode!");
			return "";
		}

		static CombineMode CombineModeFromString(const std::string& mode)
		{
			if (mode == "Average")  return CombineMode::Average;
			if (mode == "Max")     	return CombineMode::Average;
			if (mode == "Min")     	return CombineMode::Average;
			if (mode == "Multiply") return CombineMode::Average;

			VX_CORE_ASSERT(false, "Unknown Combine Mode!");
			return CombineMode::Average;
		}

	}

	SceneSerializer::SceneSerializer(const SharedRef<Scene>& scene)
		: m_Scene(scene) { }

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		size_t extensionPos = filepath.find(".");
		size_t lastSlashPos = filepath.find_last_of("/\\") + 1;
		size_t length = filepath.length();
		bool invalidFile = length == 0;

		std::string sceneName = invalidFile ? "Untitled" : filepath.substr(lastSlashPos, extensionPos - lastSlashPos);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << sceneName;
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		m_Scene->m_Registry.each([&](auto entityID)
		{
			Entity entity = { entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			VX_CORE_ERROR("Failed to load .vortex file '{}'\n     {}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		VX_CORE_TRACE("Deserializing Scene '{}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			DeserializeEntities(entities, m_Scene);
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		VX_CORE_ASSERT(entity.HasComponent<IDComponent>(), "Entity does not have a universally unique identifier!");

		const std::filesystem::path projectAssetDirectory = Project::GetAssetDirectory();

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		out << YAML::Key << "Active" << YAML::Value << entity.IsActive();

		if (entity.HasComponent<HierarchyComponent>())
		{
			const auto& hierarchyComponent = entity.GetComponent<HierarchyComponent>();
			out << YAML::Key << "Parent" << YAML::Value << hierarchyComponent.ParentUUID;

			out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;

			for (auto& child : hierarchyComponent.Children)
			{
				out << YAML::BeginMap;
				out << YAML::Key << "Handle" << YAML::Value << child;
				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
		}

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // TagComponent

			const auto& tagComponent = entity.GetComponent<TagComponent>();

			out << YAML::Key << "Tag" << YAML::Value << tagComponent.Tag;
			out << YAML::Key << "Marker" << YAML::Value << tagComponent.Marker;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent

			const auto& transform = entity.GetComponent<TransformComponent>();

			out << YAML::Key << "Translation" << YAML::Value << transform.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << transform.GetRotationEuler();
			out << YAML::Key << "Scale" << YAML::Value << transform.Scale;

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent" << YAML::BeginMap; // CameraComponent

			const auto& cameraComponent = entity.GetComponent<CameraComponent>();
			const auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOVRad();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "ClearColor" << YAML::Value << cameraComponent.ClearColor;
			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			out << YAML::Key << "SkyboxComponent";
			out << YAML::BeginMap; // SkyboxComponent

			const auto& skyboxComponent = entity.GetComponent<SkyboxComponent>();

			SharedRef<Skybox> skybox = skyboxComponent.Source;
			out << YAML::Key << "SourcePath" << std::filesystem::relative(skybox->GetFilepath(), projectAssetDirectory).string();
			out << YAML::Key << "Rotation" << skyboxComponent.Rotation;
			out << YAML::Key << "Intensity" << skyboxComponent.Intensity;

			out << YAML::EndMap; // SkyboxComponent
		}

		if (entity.HasComponent<LightSourceComponent>())
		{
			out << YAML::Key << "LightSourceComponent";
			out << YAML::BeginMap; // LightSourceComponent

			const auto& lightComponent = entity.GetComponent<LightSourceComponent>();

			out << YAML::Key << "LightType" << YAML::Value << Utils::LightTypeToString(lightComponent.Type);

			SharedRef<LightSource> lightSource = lightComponent.Source;
			out << YAML::Key << "Radiance" << YAML::Value << lightSource->GetRadiance();

			switch (lightComponent.Type)
			{
				case LightType::Directional:
				{
					break;
				}
				case LightType::Point:
				{
					break;
				}
				case LightType::Spot:
				{
					out << YAML::Key << "CutOff" << YAML::Value << lightSource->GetCutOff();
					out << YAML::Key << "OuterCutOff" << YAML::Value << lightSource->GetOuterCutOff();

					break;
				}
			}

			out << YAML::Key << "Intensity" << YAML::Value << lightSource->GetIntensity();
			out << YAML::Key << "ShadowBias" << YAML::Value << lightSource->GetShadowBias();
			out << YAML::Key << "CastShadows" << YAML::Value << lightSource->GetCastShadows();
			out << YAML::Key << "SoftShadows" << YAML::Value << lightSource->GetSoftShadows();

			out << YAML::EndMap; // LightSourceComponent
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent" << YAML::Value << YAML::BeginMap; // MeshRendererComponent

			const auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

			out << YAML::Key << "MeshType" << YAML::Value << Utils::MeshTypeToString(meshRendererComponent.Type);

			if (meshRendererComponent.Mesh)
			{
				SharedRef<Model> model = meshRendererComponent.Mesh;
				const auto& meshSourcePath = model->GetPath();
				out << YAML::Key << "MeshSource" << YAML::Value << (Model::IsDefaultMesh(meshSourcePath) ? meshSourcePath : FileSystem::Relative(meshSourcePath, projectAssetDirectory).string());

				if (ModelImportOptions importOptions = model->GetImportOptions(); importOptions != ModelImportOptions{})
				{
					out << YAML::Key << "ModelImportOptions" << YAML::Value << YAML::BeginMap; // ModelImportOptions

					out << YAML::Key << "Translation" << YAML::Value << importOptions.MeshTransformation.Translation;
					out << YAML::Key << "Rotation" << YAML::Value << importOptions.MeshTransformation.GetRotationEuler();
					out << YAML::Key << "Scale" << YAML::Value << importOptions.MeshTransformation.Scale;

					out << YAML::EndMap; // ModelImportOptions
				}

				SharedRef<Material> material = model->GetMaterial();

				SharedRef<Texture2D> albedoMap = material->GetAlbedoMap();
				SharedRef<Texture2D> normalMap = material->GetNormalMap();
				SharedRef<Texture2D> metallicMap = material->GetMetallicMap();
				SharedRef<Texture2D> roughnessMap = material->GetRoughnessMap();
				SharedRef<Texture2D> emissionMap = material->GetEmissionMap();
				SharedRef<Texture2D> parallaxOcclusionMap = material->GetParallaxOcclusionMap();
				SharedRef<Texture2D> ambientOcclusionMap = material->GetAmbientOcclusionMap();

				if (albedoMap)
					out << YAML::Key << "AlbedoMapPath" << YAML::Value << std::filesystem::relative(albedoMap->GetPath(), projectAssetDirectory).string();
				else
					out << YAML::Key << "Albedo" << YAML::Value << material->GetAlbedo();
				if (normalMap)
					out << YAML::Key << "NormalMapPath" << YAML::Value << std::filesystem::relative(normalMap->GetPath(), projectAssetDirectory).string();
				if (metallicMap)
					out << YAML::Key << "MetallicMapPath" << YAML::Value << std::filesystem::relative(metallicMap->GetPath(), projectAssetDirectory).string();
				else
					out << YAML::Key << "Metallic" << YAML::Value << material->GetMetallic();
				if (roughnessMap)
					out << YAML::Key << "RoughnessMapPath" << YAML::Value << std::filesystem::relative(roughnessMap->GetPath(), projectAssetDirectory).string();
				else
					out << YAML::Key << "Roughness" << YAML::Value << material->GetRoughness();
				if (emissionMap)
					out << YAML::Key << "EmissionMapPath" << YAML::Value << std::filesystem::relative(emissionMap->GetPath(), projectAssetDirectory).string();
				else
					out << YAML::Key << "Emission" << YAML::Value << material->GetEmission();
				if (parallaxOcclusionMap)
				{
					out << YAML::Key << "ParallaxOcclusionMapPath" << YAML::Value << std::filesystem::relative(parallaxOcclusionMap->GetPath(), projectAssetDirectory).string();
					out << YAML::Key << "ParallaxHeightScale" << YAML::Value << material->GetParallaxHeightScale();
				}
				if (ambientOcclusionMap)
					out << YAML::Key << "AmbientOcclusionMapPath" << YAML::Value << std::filesystem::relative(ambientOcclusionMap->GetPath(), projectAssetDirectory).string();

				out << YAML::Key << "UV" << YAML::Value << material->GetUV();
				out << YAML::Key << "Opacity" << YAML::Value << material->GetOpacity();
				out << YAML::Key << "MaterialFlags" << YAML::Value << material->GetFlags();
			}

			out << YAML::EndMap; // MeshRendererComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent" << YAML::Value << YAML::BeginMap; // SpriteRendererComponent

			const auto& spriteComponent = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "Color" << YAML::Value << spriteComponent.SpriteColor;
			if (spriteComponent.Texture)
				out << YAML::Key << "TexturePath" << YAML::Value << std::filesystem::relative(spriteComponent.Texture->GetPath(), projectAssetDirectory).string();
			out << YAML::Key << "TextureScale" << YAML::Value << spriteComponent.Scale;

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent" << YAML::Value << YAML::BeginMap; // CircleRendererComponent

			const auto& circleComponent = entity.GetComponent<CircleRendererComponent>();

			out << YAML::Key << "Color" << YAML::Value << circleComponent.Color;
			out << YAML::Key << "Thickness" << YAML::Value << circleComponent.Thickness;
			out << YAML::Key << "Fade" << YAML::Value << circleComponent.Fade;

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<ParticleEmitterComponent>())
		{
			out << YAML::Key << "ParticleEmitterComponent" << YAML::Value << YAML::BeginMap; // ParticleEmitterComponent

			const auto& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			const SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;

			const ParticleEmitterProperties emitterProperties = particleEmitter->GetProperties();

			out << YAML::Key << "ColorBegin" << YAML::Value << emitterProperties.ColorBegin;
			out << YAML::Key << "ColorEnd" << YAML::Value << emitterProperties.ColorEnd;
			out << YAML::Key << "LifeTime" << YAML::Value << emitterProperties.LifeTime;
			out << YAML::Key << "Position" << YAML::Value << emitterProperties.Position;
			out << YAML::Key << "Offset" << YAML::Value << emitterProperties.Offset;
			out << YAML::Key << "Rotation" << YAML::Value << emitterProperties.Rotation;
			out << YAML::Key << "SizeBegin" << YAML::Value << emitterProperties.SizeBegin;
			out << YAML::Key << "SizeEnd" << YAML::Value << emitterProperties.SizeEnd;
			out << YAML::Key << "SizeVariation" << YAML::Value << emitterProperties.SizeVariation;
			out << YAML::Key << "Velocity" << YAML::Value << emitterProperties.Velocity;
			out << YAML::Key << "VelocityVariation" << YAML::Value << emitterProperties.VelocityVariation;

			out << YAML::EndMap; // ParticleEmitterComponent
		}

		if (entity.HasComponent<TextMeshComponent>())
		{
			out << YAML::Key << "TextMeshComponent" << YAML::Value << YAML::BeginMap; // TextMeshComponent

			const auto& textMeshComponent = entity.GetComponent<TextMeshComponent>();

			// TODO come back to this
			//out << YAML::Key << "FontSourcePath" << YAML::Value << std::filesystem::relative(textMeshComponent.FontAsset->GetFontAtlas()->GetPath(), projectAssetDirectory).string();
			out << YAML::Key << "Color" << YAML::Value << textMeshComponent.Color;
			out << YAML::Key << "Kerning" << YAML::Value << textMeshComponent.Kerning;
			out << YAML::Key << "LineSpacing" << YAML::Value << textMeshComponent.LineSpacing;
			out << YAML::Key << "MaxWidth" << YAML::Value << textMeshComponent.MaxWidth;
			out << YAML::Key << "TextHash" << YAML::Value << textMeshComponent.TextHash;
			out << YAML::Key << "TextString" << YAML::Value << textMeshComponent.TextString;

			out << YAML::EndMap; // TextMeshComponent
		}

		if (entity.HasComponent<AnimatorComponent>())
		{

			out << YAML::Key << "AnimatorComponent" << YAML::Value << YAML::BeginMap; // AnimatorComponent

			const AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			

			out << YAML::EndMap; // AnimatorComponent
		}

		if (entity.HasComponent<AnimationComponent>())
		{
			out << YAML::Key << "AnimationComponent" << YAML::Value << YAML::BeginMap; // AnimationComponent

			const AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();
			SharedRef<Animation> animation = animationComponent.Animation;

			out << YAML::Key << "AnimationSourcePath" << YAML::Value << animation->GetPath();

			out << YAML::EndMap; // AnimationComponent
		}

		if (entity.HasComponent<AudioSourceComponent>())
		{
			out << YAML::Key << "AudioSourceComponent" << YAML::Value << YAML::BeginMap; // AudioSourceComponent

			const auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

			if (audioSourceComponent.Source)
			{
				const AudioSource::SoundProperties& soundProperties = audioSourceComponent.Source->GetProperties();
				
				const std::string& audioSourcePath = audioSourceComponent.Source->GetPath();
				out << YAML::Key << "AudioSourcePath" << YAML::Value << audioSourcePath;

				out << YAML::Key << "SoundSettings" << YAML::Value;
				out << YAML::BeginMap; // SoundSettings
				out << YAML::Key << "Position" << YAML::Value << soundProperties.Position;
				out << YAML::Key << "Direction" << YAML::Value << soundProperties.Direction;
				out << YAML::Key << "Veloctiy" << YAML::Value << soundProperties.Veloctiy;

				out << YAML::Key << "Cone" << YAML::Value;
				out << YAML::BeginMap; // Cone
				out << YAML::Key << "InnerAngle" << YAML::Value << soundProperties.Cone.InnerAngle;
				out << YAML::Key << "OuterAngle" << YAML::Value << soundProperties.Cone.OuterAngle;
				out << YAML::Key << "OuterGain" << YAML::Value << soundProperties.Cone.OuterGain;
				out << YAML::EndMap; // Cone

				out << YAML::Key << "MinDistance" << YAML::Value << soundProperties.MinDistance;
				out << YAML::Key << "MaxDistance" << YAML::Value << soundProperties.MaxDistance;
				out << YAML::Key << "Pitch" << YAML::Value << soundProperties.Pitch;
				out << YAML::Key << "DopplerFactor" << YAML::Value << soundProperties.DopplerFactor;
				out << YAML::Key << "Volume" << YAML::Value << soundProperties.Volume;

				out << YAML::Key << "PlayOnStart" << YAML::Value << soundProperties.PlayOnStart;
				out << YAML::Key << "PlayOneShot" << YAML::Value << soundProperties.PlayOneShot;
				out << YAML::Key << "Spacialized" << YAML::Value << soundProperties.Spacialized;
				out << YAML::Key << "Loop" << YAML::Value << soundProperties.Loop;
				out << YAML::EndMap; // SoundSettings
			}

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (entity.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent" << YAML::BeginMap; // RigidbodyComponent

			const auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "Mass" << YAML::Value << rigidbodyComponent.Mass;
			out << YAML::Key << "BodyType" << YAML::Value << Utils::RigidBodyBodyTypeToString(rigidbodyComponent.Type);
			out << YAML::Key << "AngularDrag" << YAML::Value << rigidbodyComponent.AngularDrag;
			out << YAML::Key << "AngularVelocity" << YAML::Value << rigidbodyComponent.AngularVelocity;
			out << YAML::Key << "DisableGravity" << YAML::Value << rigidbodyComponent.DisableGravity;
			out << YAML::Key << "IsKinematic" << YAML::Value << rigidbodyComponent.IsKinematic;
			out << YAML::Key << "LinearDrag" << YAML::Value << rigidbodyComponent.LinearDrag;
			out << YAML::Key << "LinearVelocity" << YAML::Value << rigidbodyComponent.LinearVelocity;
			out << YAML::Key << "CollisionDetectionType" << YAML::Value << Utils::CollisionDetectionTypeToString(rigidbodyComponent.CollisionDetection);
			out << YAML::Key << "ActorLockFlags" << YAML::Value << (uint32_t)rigidbodyComponent.LockFlags;

			out << YAML::EndMap; // RigidbodyComponent
		}

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			out << YAML::Key << "CharacterControllerComponent" << YAML::BeginMap; // CharacterControllerComponent

			const auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			out << YAML::Key << "NonWalkableMode" << Utils::NonWalkableModeToString(characterController.NonWalkMode);
			out << YAML::Key << "CapsuleClimbMode" << Utils::CapsuleClimbModeToString(characterController.ClimbMode);
			out << YAML::Key << "DisableGravity" << YAML::Key << characterController.DisableGravity;
			out << YAML::Key << "LayerID" << YAML::Key << characterController.LayerID;
			out << YAML::Key << "SlopeLimitDegrees" << YAML::Key << characterController.SlopeLimitDegrees;
			out << YAML::Key << "StepOffset" << YAML::Key << characterController.StepOffset;
			out << YAML::Key << "ContactOffset" << YAML::Key << characterController.ContactOffset;

			out << YAML::EndMap; // CharacterControllerComponent
		}

		if (entity.HasComponent<PhysicsMaterialComponent>())
		{
			out << YAML::Key << "PhysicsMaterialComponent" << YAML::BeginMap; // PhysicsMaterialComponent

			const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
			out << YAML::Key << "StaticFriction" << YAML::Key << physicsMaterial.StaticFriction;
			out << YAML::Key << "DynamicFriction" << YAML::Key << physicsMaterial.DynamicFriction;
			out << YAML::Key << "Bounciness" << YAML::Key << physicsMaterial.Bounciness;
			out << YAML::Key << "FrictionCombineMode" << YAML::Key << Utils::CombineModeToString(physicsMaterial.FrictionCombineMode);
			out << YAML::Key << "RestitutionCombineMode" << YAML::Key << Utils::CombineModeToString(physicsMaterial.RestitutionCombineMode);

			out << YAML::EndMap; // PhysicsMaterialComponent
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent" << YAML::BeginMap; // BoxColliderComponent

			const auto& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();
			out << YAML::Key << "HalfSize" << YAML::Value << boxColliderComponent.HalfSize;
			out << YAML::Key << "Offset" << YAML::Value << boxColliderComponent.Offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << boxColliderComponent.IsTrigger;

			out << YAML::EndMap; // BoxColliderComponent
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent" << YAML::BeginMap; // SphereColliderComponent

			const auto& sphereColliderComponent = entity.GetComponent<SphereColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << sphereColliderComponent.Radius;
			out << YAML::Key << "Offset" << YAML::Value << sphereColliderComponent.Offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << sphereColliderComponent.IsTrigger;

			out << YAML::EndMap; // SphereColliderComponent
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			out << YAML::Key << "CapsuleColliderComponent" << YAML::BeginMap; // CapsuleColliderComponent

			const auto& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();
			out << YAML::Key << "Radius" << YAML::Value << capsuleColliderComponent.Radius;
			out << YAML::Key << "Height" << YAML::Value << capsuleColliderComponent.Height;
			out << YAML::Key << "Offset" << YAML::Value << capsuleColliderComponent.Offset;
			out << YAML::Key << "IsTrigger" << YAML::Value << capsuleColliderComponent.IsTrigger;

			out << YAML::EndMap; // CapsuleColliderComponent
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent" << YAML::BeginMap; // Rigidbody2DComponent

			const auto& rb2dComponent = entity.GetComponent<RigidBody2DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << Utils::RigidBody2DBodyTypeToString(rb2dComponent.Type);
			out << YAML::Key << "Velocity" << YAML::Value << rb2dComponent.Velocity;
			out << YAML::Key << "Drag" << YAML::Value << rb2dComponent.Drag;
			out << YAML::Key << "AngularDrag" << YAML::Value << rb2dComponent.AngularDrag;
			out << YAML::Key << "GravityScale" << YAML::Value << rb2dComponent.GravityScale;
			out << YAML::Key << "FreezeRotation" << YAML::Value << rb2dComponent.FixedRotation;

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap; // BoxCollider2DComponent

			const auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << bc2dComponent.Size;
			out << YAML::Key << "Density" << YAML::Value << bc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.RestitutionThreshold;
			out << YAML::Key << "IsTrigger" << YAML::Value << bc2dComponent.IsTrigger;

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent" << YAML::BeginMap; // CircleCollider2DComponent

			const auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << cc2dComponent.Offset;
			out << YAML::Key << "Radius" << YAML::Value << cc2dComponent.Radius;
			out << YAML::Key << "Density" << YAML::Value << cc2dComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << cc2dComponent.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << cc2dComponent.Restitution;
			out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc2dComponent.RestitutionThreshold;

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		if (entity.HasComponent<NavMeshAgentComponent>())
		{
			out << YAML::Key << "NavMeshAgentComponent" << YAML::BeginMap; // NavMeshAgentComponent

			const auto& navMeshComponent = entity.GetComponent<NavMeshAgentComponent>();

			out << YAML::EndMap; // NavMeshAgentComponent
		}

		// TODO: This may need reworking, specifically the random empty() check
		// if the script class name is empty don't even try to serialize, just move on
		if (entity.HasComponent<ScriptComponent>() && !entity.GetComponent<ScriptComponent>().ClassName.empty())
		{
			out << YAML::Key << "ScriptComponent" << YAML::BeginMap; // ScriptComponent

			const auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			// Fields
			SharedRef<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();

			if (fields.size() > 0)
			{
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);

				out << YAML::Key << "ScriptFields" << YAML::Value;
				out << YAML::BeginSeq;

				for (const auto& [name, field] : fields)
				{
					if (entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap; // ScriptFields

					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
						WRITE_SCRIPT_FIELD(Float, float)
						WRITE_SCRIPT_FIELD(Double, double)
						WRITE_SCRIPT_FIELD(Bool, bool)
						WRITE_SCRIPT_FIELD(Char, int8_t)
						WRITE_SCRIPT_FIELD(Short, int16_t)
						WRITE_SCRIPT_FIELD(Int, int32_t)
						WRITE_SCRIPT_FIELD(Long, int64_t)
						WRITE_SCRIPT_FIELD(Byte, uint8_t)
						WRITE_SCRIPT_FIELD(UShort, uint16_t)
						WRITE_SCRIPT_FIELD(UInt, uint32_t)
						WRITE_SCRIPT_FIELD(ULong, uint64_t)
						WRITE_SCRIPT_FIELD(Vector2, Math::vec2)
						WRITE_SCRIPT_FIELD(Vector3, Math::vec3)
						WRITE_SCRIPT_FIELD(Vector4, Math::vec4)
						WRITE_SCRIPT_FIELD(Color3, Math::vec3)
						WRITE_SCRIPT_FIELD(Color4, Math::vec4)
						WRITE_SCRIPT_FIELD(Entity, UUID)
					}

					out << YAML::EndMap; // ScriptFields
				}

				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::DeserializeEntities(YAML::Node& entitiesNode, SharedRef<Scene>& scene)
	{
		for (auto entity : entitiesNode)
		{
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			bool isActive = true;
			if (entity["Active"])
				isActive = entity["Active"].as<bool>();

			std::string name;
			std::string marker;

			auto tagComponent = entity["TagComponent"];

			if (tagComponent)
			{
				if (tagComponent["Tag"])
					name = tagComponent["Tag"].as<std::string>();
				if (tagComponent["Marker"])
					marker = tagComponent["Marker"].as<std::string>();
			}

			Entity deserializedEntity = scene->CreateEntityWithUUID(uuid, name, marker);
			deserializedEntity.SetActive(isActive);

			uint64_t parentHandle = entity["Parent"] ? entity["Parent"].as<uint64_t>() : 0;
			deserializedEntity.SetParentUUID(static_cast<UUID>(parentHandle));

			const auto children = entity["Children"];

			if (children)
			{
				for (auto& child : children)
				{
					uint64_t childHandle = child["Handle"].as<uint64_t>();
					deserializedEntity.AddChild(static_cast<UUID>(childHandle));
				}
			}

			auto transformComponent = entity["TransformComponent"];
			if (transformComponent)
			{
				// All Entities have a transform
				auto& transform = deserializedEntity.GetComponent<TransformComponent>();
				transform.Translation = transformComponent["Translation"].as<Math::vec3>();
				transform.SetRotationEuler(transformComponent["Rotation"].as<Math::vec3>());
				transform.Scale = transformComponent["Scale"].as<Math::vec3>();
			}

			auto cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				auto& cc = deserializedEntity.AddComponent<CameraComponent>();

				auto cameraProps = cameraComponent["Camera"];
				cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cc.Camera.SetPerspectiveVerticalFOVRad(cameraProps["PerspectiveFOV"].as<float>());
				cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				if (cameraComponent["ClearColor"])
					cc.ClearColor = cameraComponent["ClearColor"].as<Math::vec3>();
				cc.Primary = cameraComponent["Primary"].as<bool>();
				cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
			}

			auto skyboxComponent = entity["SkyboxComponent"];
			if (skyboxComponent)
			{
				auto& skybox = deserializedEntity.AddComponent<SkyboxComponent>();

				auto path = Project::GetAssetFileSystemPath(skyboxComponent["SourcePath"].as<std::string>()).string();
				skybox.Source = Skybox::Create(path);

				if (skyboxComponent["Rotation"])
					skybox.Rotation = skyboxComponent["Rotation"].as<float>();

				if (skyboxComponent["Intensity"])
					skybox.Intensity = skyboxComponent["Intensity"].as<float>();
			}

			auto lightSourceComponent = entity["LightSourceComponent"];
			if (lightSourceComponent)
			{
				auto& lightComponent = deserializedEntity.AddComponent<LightSourceComponent>();

				lightComponent.Source = LightSource::Create(LightSourceProperties());

				lightComponent.Type = Utils::LightTypeFromString(lightSourceComponent["LightType"].as<std::string>());
				if (lightSourceComponent["Radiance"])
					lightComponent.Source->SetRadiance(lightSourceComponent["Radiance"].as<Math::vec3>());

				// Create a shadow map for the light source
				Renderer::CreateShadowMap(lightComponent.Type, lightComponent.Source);

				switch (lightComponent.Type)
				{
					case LightType::Directional:
					{
						break;
					}
					case LightType::Point:
					{
						break;
					}
					case LightType::Spot:
					{
						if (lightSourceComponent["CutOff"])
							lightComponent.Source->SetCutOff(lightSourceComponent["CutOff"].as<float>());
						if (lightSourceComponent["OuterCutOff"])
							lightComponent.Source->SetOuterCutOff(lightSourceComponent["OuterCutOff"].as<float>());

						break;
					}
				}

				if (lightSourceComponent["Intensity"])
					lightComponent.Source->SetIntensity(lightSourceComponent["Intensity"].as<float>());

				if (lightSourceComponent["ShadowBias"])
					lightComponent.Source->SetShadowBias(lightSourceComponent["ShadowBias"].as<float>());

				if (lightSourceComponent["CastShadows"])
					lightComponent.Source->SetCastShadows(lightSourceComponent["CastShadows"].as<bool>());

				if (lightSourceComponent["SoftShadows"])
					lightComponent.Source->SetSoftShadows(lightSourceComponent["SoftShadows"].as<bool>());
			}

			auto meshComponent = entity["MeshRendererComponent"];
			if (meshComponent)
			{
				auto& meshRendererComponent = deserializedEntity.AddComponent<MeshRendererComponent>();

				meshRendererComponent.Type = Utils::MeshTypeFromString(meshComponent["MeshType"].as<std::string>());

				if (meshComponent["MeshSource"])
				{
					std::string modelPath = meshComponent["MeshSource"].as<std::string>();
					std::string assetPath = "";

					ModelImportOptions importOptions = ModelImportOptions();

					if (Model::IsDefaultMesh(modelPath))
					{
						assetPath = modelPath;
					}
					else
					{
						assetPath = Project::GetAssetFileSystemPath(modelPath).string();
					}

					if (meshComponent["ModelImportOptions"])
					{
						auto modelImportOptions = meshComponent["ModelImportOptions"];
						importOptions.MeshTransformation.Translation = modelImportOptions["Translation"].as<Math::vec3>();
						importOptions.MeshTransformation.SetRotationEuler(modelImportOptions["Rotation"].as<Math::vec3>());
						importOptions.MeshTransformation.Scale = modelImportOptions["Scale"].as<Math::vec3>();
					}

					meshRendererComponent.Mesh = Model::Create(assetPath, deserializedEntity.GetTransform(), importOptions, (int)(entt::entity)deserializedEntity);
				}

				SharedRef<Material> material = meshRendererComponent.Mesh->GetMaterial() ? meshRendererComponent.Mesh->GetMaterial() : Material::Create(MaterialProperties());

				if (meshComponent["AlbedoMapPath"])
					material->SetAlbedoMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["AlbedoMapPath"].as<std::string>()).string()));
				if (meshComponent["Albedo"])
					material->SetAlbedo(meshComponent["Albedo"].as<Math::vec3>());
				if (meshComponent["NormalMapPath"])
					material->SetNormalMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["NormalMapPath"].as<std::string>()).string()));
				if (meshComponent["MetallicMapPath"])
					material->SetMetallicMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["MetallicMapPath"].as<std::string>()).string()));
				if (meshComponent["Metallic"])
					material->SetMetallic(meshComponent["Metallic"].as<float>());
				if (meshComponent["RoughnessMapPath"])
					material->SetRoughnessMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["RoughnessMapPath"].as<std::string>()).string()));
				if (meshComponent["Roughness"])
					material->SetRoughness(meshComponent["Roughness"].as<float>());
				if (meshComponent["EmissionMapPath"])
					material->SetEmissionMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["EmissionMapPath"].as<std::string>()).string()));
				if (meshComponent["Emission"])
					material->SetEmission(meshComponent["Emission"].as<float>());
				if (meshComponent["ParallaxOcclusionMapPath"])
					material->SetParallaxOcclusionMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["ParallaxOcclusionMapPath"].as<std::string>()).string()));
				if (meshComponent["ParallaxHeightScale"])
					material->SetParallaxHeightScale(meshComponent["ParallaxHeightScale"].as<float>());
				if (meshComponent["AmbientOcclusionMapPath"])
					material->SetAmbientOcclusionMap(Texture2D::Create(Project::GetAssetFileSystemPath(meshComponent["AmbientOcclusionMapPath"].as<std::string>()).string()));

				if (meshComponent["UV"])
					material->SetUV(meshComponent["UV"].as<Math::vec2>());

				if (meshComponent["Opacity"])
					material->SetOpacity(meshComponent["Opacity"].as<float>());

				if (meshComponent["MaterialFlags"])
					material->SetFlags(meshComponent["MaterialFlags"].as<uint32_t>());
			}

			auto spriteComponent = entity["SpriteRendererComponent"];
			if (spriteComponent)
			{
				auto& spriteRendererComponent = deserializedEntity.AddComponent<SpriteRendererComponent>();

				spriteRendererComponent.SpriteColor = spriteComponent["Color"].as<Math::vec4>();

				if (spriteComponent["TexturePath"])
					spriteRendererComponent.Texture = Texture2D::Create(Project::GetAssetFileSystemPath(spriteComponent["TexturePath"].as<std::string>()).string());

				if (spriteComponent["TextureScale"])
					spriteRendererComponent.Scale = spriteComponent["TextureScale"].as<Math::vec2>();
			}

			auto circleComponent = entity["CircleRendererComponent"];
			if (circleComponent)
			{
				auto& circleRendererComponent = deserializedEntity.AddComponent<CircleRendererComponent>();

				circleRendererComponent.Color = circleComponent["Color"].as<Math::vec4>();
				circleRendererComponent.Thickness = circleComponent["Thickness"].as<float>();
				circleRendererComponent.Fade = circleComponent["Fade"].as<float>();
			}

			auto particleEmitterComponent = entity["ParticleEmitterComponent"];
			if (particleEmitterComponent)
			{
				auto& particleEmitter = deserializedEntity.AddComponent<ParticleEmitterComponent>();

				ParticleEmitterProperties emitterProperties;
				emitterProperties.ColorBegin = particleEmitterComponent["ColorBegin"].as<Math::vec4>();
				emitterProperties.ColorEnd = particleEmitterComponent["ColorEnd"].as<Math::vec4>();
				emitterProperties.LifeTime = particleEmitterComponent["LifeTime"].as<float>();
				emitterProperties.Position = particleEmitterComponent["Position"].as<Math::vec3>();
				if (particleEmitterComponent["Offset"])
					emitterProperties.Offset = particleEmitterComponent["Offset"].as<Math::vec3>();
				emitterProperties.Rotation = particleEmitterComponent["Rotation"].as<float>();
				emitterProperties.SizeBegin = particleEmitterComponent["SizeBegin"].as<Math::vec2>();
				emitterProperties.SizeEnd = particleEmitterComponent["SizeEnd"].as<Math::vec2>();
				emitterProperties.SizeVariation = particleEmitterComponent["SizeVariation"].as<Math::vec2>();
				emitterProperties.Velocity = particleEmitterComponent["Velocity"].as<Math::vec3>();
				emitterProperties.VelocityVariation = particleEmitterComponent["VelocityVariation"].as<Math::vec3>();

				particleEmitter.Emitter = ParticleEmitter::Create(emitterProperties);
			}

			auto textMeshComponent = entity["TextMeshComponent"];
			if (textMeshComponent)
			{
				auto& tmc = deserializedEntity.AddComponent<TextMeshComponent>();

				//tmc.FontAsset = Font::Create(Project::GetAssetFileSystemPath(textMeshComponent["FontSourcePath"].as<std::string>()));
				tmc.Color = textMeshComponent["Color"].as<Math::vec4>();
				tmc.Kerning = textMeshComponent["Kerning"].as<float>();
				tmc.LineSpacing = textMeshComponent["LineSpacing"].as<float>();
				tmc.MaxWidth = textMeshComponent["MaxWidth"].as<float>();
				tmc.TextHash = textMeshComponent["TextHash"].as<size_t>();
				tmc.TextString = textMeshComponent["TextString"].as<std::string>();
			}

			auto animationComponent = entity["AnimationComponent"];
			if (animationComponent)
			{
				if (!deserializedEntity.HasComponent<MeshRendererComponent>())
				{
					VX_CORE_WARN("Trying to add Animation Component without Mesh Renderer Component!");
					return;
				}

				auto& animation = deserializedEntity.AddComponent<AnimationComponent>();
				SharedRef<Model> model = deserializedEntity.GetComponent<MeshRendererComponent>().Mesh;
				std::string filepath = model->GetPath();
				animation.Animation = Animation::Create(filepath, model);
			}

			auto animatorComponent = entity["AnimatorComponent"];
			if (animatorComponent)
			{
				if (!deserializedEntity.HasComponent<AnimationComponent>())
				{
					VX_CORE_WARN("Trying to add Animator Component without Animation Component!");
					return;
				}

				auto& animator = deserializedEntity.AddComponent<AnimatorComponent>();
				animator.Animator = Animator::Create(deserializedEntity.GetComponent<AnimationComponent>().Animation);
			}

			auto audioSourceComponent = entity["AudioSourceComponent"];
			if (audioSourceComponent)
			{
				auto& asc = deserializedEntity.AddComponent<AudioSourceComponent>();

				if (audioSourceComponent["AudioSourcePath"])
				{
					std::string audioSourcePath= audioSourceComponent["AudioSourcePath"].as<std::string>();
					if (FileSystem::Exists(audioSourcePath))
					{
						asc.Source = AudioSource::Create(audioSourcePath);
					}
				}

				auto soundProps = audioSourceComponent["SoundSettings"];

				if (soundProps)
				{
					auto& soundProperties = asc.Source->GetProperties();
					if (soundProps["Position"])
						soundProperties.Position = soundProps["Position"].as<Math::vec3>();
					if (soundProps["Direction"])
						soundProperties.Direction = soundProps["Direction"].as<Math::vec3>();
					if (soundProps["Velocity"])
						soundProperties.Veloctiy = soundProps["Velocity"].as<Math::vec3>();

					if (soundProps["Cone"])
					{
						auto cone = soundProps["Cone"];
						soundProperties.Cone.InnerAngle = cone["InnerAngle"].as<float>();
						soundProperties.Cone.OuterAngle = cone["OuterAngle"].as<float>();
						soundProperties.Cone.OuterGain = cone["OuterGain"].as<float>();
					}

					if (soundProps["MinDistance"])
						soundProperties.MinDistance = soundProps["MinDistance"].as<float>();
					if (soundProps["MaxDistance"])
						soundProperties.MaxDistance = soundProps["MaxDistance"].as<float>();

					if (soundProps["Pitch"])
						soundProperties.Pitch = soundProps["Pitch"].as<float>();
					if (soundProps["DopplerFactor"])
						soundProperties.DopplerFactor = soundProps["DopplerFactor"].as<float>();
					if (soundProps["Volume"])
						soundProperties.Volume = soundProps["Volume"].as<float>();

					if (soundProps["PlayOnStart"])
						soundProperties.PlayOnStart = soundProps["PlayOnStart"].as<bool>();
					if (soundProps["PlayOneShot"])
						soundProperties.PlayOneShot = soundProps["PlayOneShot"].as<bool>();
					if (soundProps["Spacialized"])
						soundProperties.Spacialized = soundProps["Spacialized"].as<bool>();
					if (soundProps["Loop"])
						soundProperties.Loop = soundProps["Loop"].as<bool>();

					asc.Source->SetProperties(soundProperties);
				}
			}

			auto rigidbodyComponent = entity["RigidbodyComponent"];
			if (rigidbodyComponent)
			{
				auto& rigidbody = deserializedEntity.AddComponent<RigidBodyComponent>();

				rigidbody.Type = Utils::RigidBodyBodyTypeFromString(rigidbodyComponent["BodyType"].as<std::string>());
				if (rigidbodyComponent["Mass"])
					rigidbody.Mass = rigidbodyComponent["Mass"].as<float>();
				if (rigidbodyComponent["AngularDrag"])
					rigidbody.AngularDrag = rigidbodyComponent["AngularDrag"].as<float>();
				if (rigidbodyComponent["AngularVelocity"])
					rigidbody.AngularVelocity = rigidbodyComponent["AngularVelocity"].as<Math::vec3>();
				if (rigidbodyComponent["DisableGravity"])
					rigidbody.DisableGravity = rigidbodyComponent["DisableGravity"].as<bool>();
				if (rigidbodyComponent["IsKinematic"])
					rigidbody.IsKinematic = rigidbodyComponent["IsKinematic"].as<bool>();
				if (rigidbodyComponent["LinearDrag"])
					rigidbody.LinearDrag = rigidbodyComponent["LinearDrag"].as<float>();
				if (rigidbodyComponent["LinearVelocity"])
					rigidbody.LinearVelocity = rigidbodyComponent["LinearVelocity"].as<Math::vec3>();
				if (rigidbodyComponent["CollisionDetectionType"])
					rigidbody.CollisionDetection = Utils::CollisionDetectionTypeFromString(rigidbodyComponent["CollisionDetectionType"].as<std::string>());
				if (rigidbodyComponent["ActorLockFlags"])
					rigidbody.LockFlags = rigidbodyComponent["ActorLockFlags"].as<uint32_t>(0);
			}

			auto characterControllerComponent = entity["CharacterControllerComponent"];
			if (characterControllerComponent)
			{
				auto& characterController = deserializedEntity.AddComponent<CharacterControllerComponent>();

				if (characterControllerComponent["NonWalkableMode"])
					characterController.NonWalkMode = Utils::NonWalkableModeFromString(characterControllerComponent["NonWalkableMode"].as<std::string>());
				if (characterControllerComponent["CapsuleClimbMode"])
					characterController.ClimbMode = Utils::CapsuleClimbModeFromString(characterControllerComponent["CapsuleClimbMode"].as<std::string>());
				characterController.DisableGravity = characterControllerComponent["DisableGravity"].as<bool>();
				characterController.LayerID = characterControllerComponent["LayerID"].as<uint32_t>();
				characterController.SlopeLimitDegrees = characterControllerComponent["SlopeLimitDegrees"].as<float>();
				characterController.StepOffset = characterControllerComponent["StepOffset"].as<float>();
				if (characterControllerComponent["ContactOffset"])
					characterController.ContactOffset = characterControllerComponent["ContactOffset"].as<float>();
			}

			auto physicsMaterialComponent = entity["PhysicsMaterialComponent"];
			if (physicsMaterialComponent)
			{
				auto& physicsMaterial = deserializedEntity.AddComponent<PhysicsMaterialComponent>();

				physicsMaterial.StaticFriction = physicsMaterialComponent["StaticFriction"].as<float>();
				physicsMaterial.DynamicFriction = physicsMaterialComponent["DynamicFriction"].as<float>();
				physicsMaterial.Bounciness = physicsMaterialComponent["Bounciness"].as<float>();
				if (physicsMaterialComponent["FrictionCombineMode"])
					physicsMaterial.FrictionCombineMode = Utils::CombineModeFromString(physicsMaterialComponent["FrictionCombineMode"].as<std::string>());
				if (physicsMaterialComponent["RestitutionCombineMode"])
					physicsMaterial.RestitutionCombineMode = Utils::CombineModeFromString(physicsMaterialComponent["RestitutionCombineMode"].as<std::string>());
			}

			auto boxColliderComponent = entity["BoxColliderComponent"];
			if (boxColliderComponent)
			{
				auto& boxCollider = deserializedEntity.AddComponent<BoxColliderComponent>();

				boxCollider.HalfSize = boxColliderComponent["HalfSize"].as<glm::vec3>();
				boxCollider.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
				if (boxColliderComponent["IsTrigger"])
					boxCollider.IsTrigger = boxColliderComponent["IsTrigger"].as<bool>();
			}

			auto sphereColliderComponent = entity["SphereColliderComponent"];
			if (sphereColliderComponent)
			{
				auto& sphereCollider = deserializedEntity.AddComponent<SphereColliderComponent>();

				sphereCollider.Radius = sphereColliderComponent["Radius"].as<float>();
				sphereCollider.Offset = sphereColliderComponent["Offset"].as<Math::vec3>();
				sphereCollider.IsTrigger = sphereColliderComponent["IsTrigger"].as<bool>();
			}

			auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
			if (capsuleColliderComponent)
			{
				auto& capsuleCollider = deserializedEntity.AddComponent<CapsuleColliderComponent>();

				capsuleCollider.Radius = capsuleColliderComponent["Radius"].as<float>();
				capsuleCollider.Height = capsuleColliderComponent["Height"].as<float>();
				capsuleCollider.Offset = capsuleColliderComponent["Offset"].as<Math::vec3>();
				capsuleCollider.IsTrigger = capsuleColliderComponent["IsTrigger"].as<bool>();
			}

			auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
			if (rigidbody2DComponent)
			{
				auto& rb2d = deserializedEntity.AddComponent<RigidBody2DComponent>();

				rb2d.Type = Utils::RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
				if (rigidbody2DComponent["Velocity"])
					rb2d.Velocity = rigidbody2DComponent["Velocity"].as<Math::vec2>();
				if (rigidbody2DComponent["Drag"])
					rb2d.Drag = rigidbody2DComponent["Drag"].as<float>();
				if (rigidbody2DComponent["AngularDrag"])
					rb2d.AngularDrag = rigidbody2DComponent["AngularDrag"].as<float>();
				if (rigidbody2DComponent["GravityScale"])
					rb2d.GravityScale = rigidbody2DComponent["GravityScale"].as<float>();
				if (rigidbody2DComponent["FreezeRotation"])
					rb2d.FixedRotation = rigidbody2DComponent["FreezeRotation"].as<bool>();
			}

			auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
			if (boxCollider2DComponent)
			{
				auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();

				bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
				bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
				bc2d.Density = boxCollider2DComponent["Density"].as<float>();
				bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
				bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
				bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				if (boxCollider2DComponent["IsTrigger"])
					bc2d.IsTrigger = boxCollider2DComponent["IsTrigger"].as<bool>();
			}

			auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
			if (circleCollider2DComponent)
			{
				auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();

				cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
				cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
				cc2d.Density = circleCollider2DComponent["Density"].as<float>();
				cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
				cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
				cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
			}

			auto scriptComponent = entity["ScriptComponent"];
			if (scriptComponent)
			{
				auto& sc = deserializedEntity.AddComponent<ScriptComponent>();
				sc.ClassName = scriptComponent["ClassName"].as<std::string>();

				auto scriptFields = scriptComponent["ScriptFields"];
				if (scriptFields)
				{
					SharedRef<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
					if (entityClass)
					{
						const auto& fields = entityClass->GetFields();
						auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string typeString = scriptField["Type"].as<std::string>();
							ScriptFieldType type = Utils::StringToScriptFieldType(typeString);

							ScriptFieldInstance& fieldInstance = entityFields[name];

							if (fields.find(name) == fields.end())
							{
								VX_CORE_WARN("Script Field '{}' was not found in Field Map!", name);
								continue;
							}

							fieldInstance.Field = fields.at(name);

							switch (type)
							{
								READ_SCRIPT_FIELD(Float, float)
								READ_SCRIPT_FIELD(Double, double)
								READ_SCRIPT_FIELD(Bool, bool)
								READ_SCRIPT_FIELD(Char, int8_t)
								READ_SCRIPT_FIELD(Short, int16_t)
								READ_SCRIPT_FIELD(Int, int32_t)
								READ_SCRIPT_FIELD(Long, int64_t)
								READ_SCRIPT_FIELD(Byte, uint8_t)
								READ_SCRIPT_FIELD(UShort, uint16_t)
								READ_SCRIPT_FIELD(UInt, uint32_t)
								READ_SCRIPT_FIELD(ULong, uint64_t)
								READ_SCRIPT_FIELD(Vector2, Math::vec2)
								READ_SCRIPT_FIELD(Vector3, Math::vec3)
								READ_SCRIPT_FIELD(Vector4, Math::vec4)
								READ_SCRIPT_FIELD(Color3, Math::vec3)
								READ_SCRIPT_FIELD(Color4, Math::vec4)
								READ_SCRIPT_FIELD(Entity, UUID)
							}
						}
					}
				}
			}
		}
	}

}
