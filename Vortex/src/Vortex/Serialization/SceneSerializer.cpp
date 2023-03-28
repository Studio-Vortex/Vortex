#include "vxpch.h"
#include "SceneSerializer.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Asset/AssetManager.h"
#include "Vortex/Asset/AssetImporter.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/Font/Font.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"

#include "Vortex/Editor/EditorResources.h"

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

		static std::string AttenuationModelTypeToString(AttenuationModel attenuationModel)
		{
			switch (attenuationModel)
			{
				case AttenuationModel::None:        return "None";
				case AttenuationModel::Inverse:     return "Inverse";
				case AttenuationModel::Linear:      return "Linear";
				case AttenuationModel::Exponential: return "Exponential";
			}

			VX_CORE_ASSERT(false, "Unknown Attenutation Model!");
			return "None";
		}

		static AttenuationModel AttenuationModelTypeFromString(const std::string& attenuationModel)
		{
			if (attenuationModel == "None")        return AttenuationModel::None;
			if (attenuationModel == "Inverse")     return AttenuationModel::Inverse;
			if (attenuationModel == "Linear")      return AttenuationModel::Linear;
			if (attenuationModel == "Exponential") return AttenuationModel::Exponential;

			VX_CORE_ASSERT(false, "Unknown Attenutation Model!");
			return AttenuationModel::None;
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

		static std::string RigidBodyTypeToString(RigidBodyType bodyType)
		{
			switch (bodyType)
			{
				case RigidBodyType::Static:    return "Static";
				case RigidBodyType::Dynamic:   return "Dynamic";
			}

			VX_CORE_ASSERT(false, "Unknown Body Type!");
			return {};
		}

		static RigidBodyType RigidBodyTypeFromString(const std::string& bodyTypeString)
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

	SceneSerializer::SceneSerializer(const SharedReference<Scene>& scene)
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
			Entity entity = { entityID, m_Scene.Raw() };

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
			VX_CONSOLE_LOG_ERROR("Failed to load .vortex file '{}'\n     {}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		VX_CONSOLE_LOG_TRACE("Deserializing Scene '{}'", sceneName);

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

		VX_SERIALIZE_PROPERTY(Entity, entity.GetUUID(), out);
		VX_SERIALIZE_PROPERTY(Active, entity.IsActive(), out);

		if (entity.HasComponent<HierarchyComponent>())
		{
			const auto& hierarchyComponent = entity.GetComponent<HierarchyComponent>();
			VX_SERIALIZE_PROPERTY(Parent, hierarchyComponent.ParentUUID, out);

			out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;

			for (const auto& child : hierarchyComponent.Children)
			{
				out << YAML::BeginMap;
				VX_SERIALIZE_PROPERTY(Handle, child, out);
				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
		}

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // TagComponent

			const auto& tagComponent = entity.GetComponent<TagComponent>();

			VX_SERIALIZE_PROPERTY(Tag, tagComponent.Tag, out);
			VX_SERIALIZE_PROPERTY(Marker, tagComponent.Marker, out);

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent

			const auto& transform = entity.GetComponent<TransformComponent>();

			VX_SERIALIZE_PROPERTY(Translation, transform.Translation, out);
			VX_SERIALIZE_PROPERTY(Rotation, transform.GetRotationEuler(), out);
			VX_SERIALIZE_PROPERTY(Scale, transform.Scale, out);

			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent" << YAML::BeginMap; // CameraComponent

			const auto& cameraComponent = entity.GetComponent<CameraComponent>();
			const auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;

			out << YAML::BeginMap; // Camera
			VX_SERIALIZE_PROPERTY(ProjectionType, (int)camera.GetProjectionType(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveFOV, camera.GetPerspectiveVerticalFOVRad(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveNear, camera.GetPerspectiveNearClip(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveFar, camera.GetPerspectiveFarClip(), out);
			VX_SERIALIZE_PROPERTY(OrthographicSize, camera.GetOrthographicSize(), out);
			VX_SERIALIZE_PROPERTY(OrthographicNear, camera.GetOrthographicNearClip(), out);
			VX_SERIALIZE_PROPERTY(OrthographicFar, camera.GetOrthographicFarClip(), out);
			out << YAML::EndMap; // Camera

			VX_SERIALIZE_PROPERTY(ClearColor, cameraComponent.ClearColor, out);
			VX_SERIALIZE_PROPERTY(Primary, cameraComponent.Primary, out);
			VX_SERIALIZE_PROPERTY(FixedAspectRatio, cameraComponent.FixedAspectRatio, out);

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SkyboxComponent>())
		{
			const auto& skyboxComponent = entity.GetComponent<SkyboxComponent>();
			AssetHandle environmentHandle = skyboxComponent.Skybox;
			
			out << YAML::Key << "SkyboxComponent" << YAML::BeginMap; // SkyboxComponent

			VX_SERIALIZE_PROPERTY(Skybox, environmentHandle, out);
			VX_SERIALIZE_PROPERTY(Rotation, skyboxComponent.Rotation, out);
			VX_SERIALIZE_PROPERTY(Intensity, skyboxComponent.Intensity, out);

			out << YAML::EndMap; // SkyboxComponent
		}

		if (entity.HasComponent<LightSourceComponent>())
		{
			out << YAML::Key << "LightSourceComponent";
			out << YAML::BeginMap; // LightSourceComponent

			const auto& lightComponent = entity.GetComponent<LightSourceComponent>();

			VX_SERIALIZE_PROPERTY(Visible, lightComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(LightType, Utils::LightTypeToString(lightComponent.Type), out);

			VX_SERIALIZE_PROPERTY(Radiance, lightComponent.Radiance, out);

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
					VX_SERIALIZE_PROPERTY(CutOff, lightComponent.Cutoff,  out);
					VX_SERIALIZE_PROPERTY(OuterCutOff, lightComponent.OuterCutoff, out);

					break;
				}
			}

			VX_SERIALIZE_PROPERTY(Intensity, lightComponent.Intensity, out);
			VX_SERIALIZE_PROPERTY(ShadowBias, lightComponent.ShadowBias, out);
			VX_SERIALIZE_PROPERTY(CastShadows, lightComponent.CastShadows, out);
			VX_SERIALIZE_PROPERTY(SoftShadows, lightComponent.SoftShadows, out);

			out << YAML::EndMap; // LightSourceComponent
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			const auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (AssetManager::IsHandleValid(meshHandle))
			{
				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
				if (mesh)
				{
					out << YAML::Key << "MeshRendererComponent" << YAML::Value << YAML::BeginMap; // MeshRendererComponent

					VX_SERIALIZE_PROPERTY(MeshHandle, meshHandle, out);
					VX_SERIALIZE_PROPERTY(Visible, meshRendererComponent.Visible, out);

					if (MeshImportOptions importOptions = mesh->GetImportOptions(); importOptions != MeshImportOptions{})
					{
						out << YAML::Key << "MeshImportOptions" << YAML::Value << YAML::BeginMap; // MeshImportOptions

						VX_SERIALIZE_PROPERTY(Translation, importOptions.MeshTransformation.Translation, out);
						VX_SERIALIZE_PROPERTY(Rotation, importOptions.MeshTransformation.GetRotationEuler(), out);
						VX_SERIALIZE_PROPERTY(Scale, importOptions.MeshTransformation.Scale, out);

						out << YAML::EndMap; // MeshImportOptions
					}

					{
						const auto& submesh = mesh->GetSubmesh();

						out << YAML::BeginMap; // Submesh

						VX_SERIALIZE_PROPERTY(Name, submesh.GetName(), out);

						// this should be switched to an asset handle
						//SharedReference<Material> material = submesh.GetMaterial();

						//AssetImporter::Serialize(material);

						out << YAML::EndMap; // Submesh
					}

					out << YAML::EndMap; // MeshRendererComponent
				}
			}
		}

		if (entity.HasComponent<StaticMeshRendererComponent>())
		{
			const auto& staticMeshRenderer = entity.GetComponent<StaticMeshRendererComponent>();
			AssetHandle staticMeshHandle = staticMeshRenderer.StaticMesh;
			if (AssetManager::IsHandleValid(staticMeshHandle))
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
				if (staticMesh)
				{
					out << YAML::Key << "StaticMeshRendererComponent" << YAML::Value << YAML::BeginMap; // StaticMeshRendererComponent

					VX_SERIALIZE_PROPERTY(MeshHandle, staticMeshHandle, out);
					VX_SERIALIZE_PROPERTY(MeshType, Utils::MeshTypeToString(staticMeshRenderer.Type), out);
					VX_SERIALIZE_PROPERTY(Visible, staticMeshRenderer.Visible, out);

					if (MeshImportOptions importOptions = staticMesh->GetImportOptions(); importOptions != MeshImportOptions{})
					{
						out << YAML::Key << "MeshImportOptions" << YAML::Value << YAML::BeginMap; // MeshImportOptions

						VX_SERIALIZE_PROPERTY(Translation, importOptions.MeshTransformation.Translation, out);
						VX_SERIALIZE_PROPERTY(Rotation, importOptions.MeshTransformation.GetRotationEuler(), out);
						VX_SERIALIZE_PROPERTY(Scale, importOptions.MeshTransformation.Scale, out);

						out << YAML::EndMap; // MeshImportOptions
					}

					out << YAML::Key << "Submeshes" << YAML::Value << YAML::BeginSeq;
					{
						const auto& submeshes = staticMesh->GetSubmeshes();

						for (const auto& [submeshIndex, submesh] :submeshes)
						{
							out << YAML::BeginMap; // Submesh

							VX_SERIALIZE_PROPERTY(Name, submesh.GetName(), out);

							AssetHandle materialHandle = staticMeshRenderer.Materials->GetMaterial(submeshIndex);
							if (!AssetManager::IsHandleValid(materialHandle))
								continue;

							SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
							if (!material)
								continue;

							AssetImporter::Serialize(material);

							VX_SERIALIZE_PROPERTY(MaterialHandle, materialHandle, out);

							out << YAML::EndMap; // Submesh
						}
					}
					out << YAML::EndSeq; // Submeshes
					
					out << YAML::EndMap; // StaticMeshRendererComponent
				}
			}
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			const auto& spriteComponent = entity.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent" << YAML::Value << YAML::BeginMap; // SpriteRendererComponent

			AssetHandle spriteHandle = spriteComponent.Texture;
			VX_SERIALIZE_PROPERTY(TextureHandle, spriteHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, spriteComponent.Visible, out);

			VX_SERIALIZE_PROPERTY(Color, spriteComponent.SpriteColor, out);
			VX_SERIALIZE_PROPERTY(TextureUV, spriteComponent.TextureUV, out);

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (entity.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent" << YAML::Value << YAML::BeginMap; // CircleRendererComponent

			const auto& circleComponent = entity.GetComponent<CircleRendererComponent>();

			VX_SERIALIZE_PROPERTY(Visible, circleComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, circleComponent.Color, out);
			VX_SERIALIZE_PROPERTY(Thickness, circleComponent.Thickness, out);
			VX_SERIALIZE_PROPERTY(Fade, circleComponent.Fade, out);

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (entity.HasComponent<ParticleEmitterComponent>())
		{
			out << YAML::Key << "ParticleEmitterComponent" << YAML::Value << YAML::BeginMap; // ParticleEmitterComponent

			const auto& particleEmitterComponent = entity.GetComponent<ParticleEmitterComponent>();
			const SharedRef<ParticleEmitter> particleEmitter = particleEmitterComponent.Emitter;

			const ParticleEmitterProperties emitterProperties = particleEmitter->GetProperties();

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

			out << YAML::EndMap; // ParticleEmitterComponent
		}

		if (entity.HasComponent<TextMeshComponent>())
		{
			const auto& textMeshComponent = entity.GetComponent<TextMeshComponent>();

			out << YAML::Key << "TextMeshComponent" << YAML::Value << YAML::BeginMap; // TextMeshComponent

			AssetHandle fontHandle = textMeshComponent.FontAsset;

			VX_SERIALIZE_PROPERTY(FontHandle, fontHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, textMeshComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, textMeshComponent.Color, out);
			VX_SERIALIZE_PROPERTY(BgColor, textMeshComponent.BgColor, out);
			VX_SERIALIZE_PROPERTY(Kerning, textMeshComponent.Kerning, out);
			VX_SERIALIZE_PROPERTY(LineSpacing, textMeshComponent.LineSpacing, out);
			VX_SERIALIZE_PROPERTY(MaxWidth, textMeshComponent.MaxWidth, out);
			VX_SERIALIZE_PROPERTY(TextHash, textMeshComponent.TextHash, out);
			VX_SERIALIZE_PROPERTY(TextString, textMeshComponent.TextString, out);

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

			VX_SERIALIZE_PROPERTY(AnimationSourcePath, animation->GetPath(), out);

			out << YAML::EndMap; // AnimationComponent
		}

		if (entity.HasComponent<AudioSourceComponent>())
		{
			out << YAML::Key << "AudioSourceComponent" << YAML::Value << YAML::BeginMap; // AudioSourceComponent

			const auto& audioSourceComponent = entity.GetComponent<AudioSourceComponent>();

			if (audioSourceComponent.Source)
			{
				const PlaybackDeviceProperties& deviceProperties = audioSourceComponent.Source->GetProperties();
				
				const std::string& audioSourcePath = audioSourceComponent.Source->GetPath();
				VX_SERIALIZE_PROPERTY(AudioSourcePath, audioSourcePath, out);

				out << YAML::Key << "SoundSettings" << YAML::Value;
				out << YAML::BeginMap; // SoundSettings
				VX_SERIALIZE_PROPERTY(Position, deviceProperties.Position, out);
				VX_SERIALIZE_PROPERTY(Direction, deviceProperties.Direction, out);
				VX_SERIALIZE_PROPERTY(Velocity, deviceProperties.Velocity, out);

				out << YAML::Key << "Cone" << YAML::Value;
				out << YAML::BeginMap; // Cone
				VX_SERIALIZE_PROPERTY(InnerAngle, deviceProperties.Cone.InnerAngle, out);
				VX_SERIALIZE_PROPERTY(OuterAngle, deviceProperties.Cone.OuterAngle, out);
				VX_SERIALIZE_PROPERTY(OuterGain, deviceProperties.Cone.OuterGain, out);
				out << YAML::EndMap; // Cone

				VX_SERIALIZE_PROPERTY(MinGain, deviceProperties.MinGain, out);
				VX_SERIALIZE_PROPERTY(MaxGain, deviceProperties.MaxGain, out);

				VX_SERIALIZE_PROPERTY(AttenuationModel, Utils::AttenuationModelTypeToString(deviceProperties.AttenuationModel), out);
				VX_SERIALIZE_PROPERTY(Falloff, deviceProperties.Falloff, out);

				VX_SERIALIZE_PROPERTY(MinDistance, deviceProperties.MinDistance, out);
				VX_SERIALIZE_PROPERTY(MaxDistance, deviceProperties.MaxDistance, out);
				VX_SERIALIZE_PROPERTY(Pitch, deviceProperties.Pitch, out);
				VX_SERIALIZE_PROPERTY(DopplerFactor, deviceProperties.DopplerFactor, out);
				VX_SERIALIZE_PROPERTY(Volume, deviceProperties.Volume, out);

				VX_SERIALIZE_PROPERTY(PlayOnStart, deviceProperties.PlayOnStart, out);
				VX_SERIALIZE_PROPERTY(PlayOneShot, deviceProperties.PlayOneShot, out);
				VX_SERIALIZE_PROPERTY(Spacialized, deviceProperties.Spacialized, out);
				VX_SERIALIZE_PROPERTY(Loop, deviceProperties.Loop, out);
				out << YAML::EndMap; // SoundSettings
			}

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (entity.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent" << YAML::BeginMap; // RigidbodyComponent

			const auto& rigidbodyComponent = entity.GetComponent<RigidBodyComponent>();
			VX_SERIALIZE_PROPERTY(Mass, rigidbodyComponent.Mass, out);
			VX_SERIALIZE_PROPERTY(BodyType, Utils::RigidBodyTypeToString(rigidbodyComponent.Type), out);
			VX_SERIALIZE_PROPERTY(AngularDrag, rigidbodyComponent.AngularDrag, out);
			VX_SERIALIZE_PROPERTY(MaxAngularVelocity, rigidbodyComponent.MaxAngularVelocity, out);
			VX_SERIALIZE_PROPERTY(AngularVelocity, rigidbodyComponent.AngularVelocity, out);
			VX_SERIALIZE_PROPERTY(DisableGravity, rigidbodyComponent.DisableGravity, out);
			VX_SERIALIZE_PROPERTY(IsKinematic, rigidbodyComponent.IsKinematic, out);
			VX_SERIALIZE_PROPERTY(LinearDrag, rigidbodyComponent.LinearDrag, out);
			VX_SERIALIZE_PROPERTY(MaxLinearVelocity, rigidbodyComponent.MaxLinearVelocity, out);
			VX_SERIALIZE_PROPERTY(LinearVelocity, rigidbodyComponent.LinearVelocity, out);
			VX_SERIALIZE_PROPERTY(CollisionDetectionType, Utils::CollisionDetectionTypeToString(rigidbodyComponent.CollisionDetection), out);
			VX_SERIALIZE_PROPERTY(ActorLockFlags, (uint32_t)rigidbodyComponent.LockFlags, out);

			out << YAML::EndMap; // RigidbodyComponent
		}

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			out << YAML::Key << "CharacterControllerComponent" << YAML::BeginMap; // CharacterControllerComponent

			const auto& characterController = entity.GetComponent<CharacterControllerComponent>();
			VX_SERIALIZE_PROPERTY(NonWalkableMode, Utils::NonWalkableModeToString(characterController.NonWalkMode), out);
			VX_SERIALIZE_PROPERTY(CapsuleClimbMode, Utils::CapsuleClimbModeToString(characterController.ClimbMode), out);
			VX_SERIALIZE_PROPERTY(DisableGravity, characterController.DisableGravity, out);
			VX_SERIALIZE_PROPERTY(LayerID, characterController.LayerID, out);
			VX_SERIALIZE_PROPERTY(SlopeLimitDegrees, characterController.SlopeLimitDegrees, out);
			VX_SERIALIZE_PROPERTY(StepOffset, characterController.StepOffset, out);
			VX_SERIALIZE_PROPERTY(ContactOffset, characterController.ContactOffset, out);

			out << YAML::EndMap; // CharacterControllerComponent
		}

		if (entity.HasComponent<FixedJointComponent>())
		{
			out << YAML::Key << "FixedJointComponent" << YAML::BeginMap; // FixedJointComponent

			const auto& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
			VX_SERIALIZE_PROPERTY(ConnectedEntity, fixedJointComponent.ConnectedEntity, out);
			VX_SERIALIZE_PROPERTY(BreakForce, fixedJointComponent.BreakForce, out);
			VX_SERIALIZE_PROPERTY(BreakTorque, fixedJointComponent.BreakTorque, out);
			VX_SERIALIZE_PROPERTY(EnableCollision, fixedJointComponent.EnableCollision, out);
			VX_SERIALIZE_PROPERTY(EnablePreProcessing, fixedJointComponent.EnablePreProcessing, out);
			VX_SERIALIZE_PROPERTY(IsBreakable, fixedJointComponent.IsBreakable, out);

			out << YAML::EndMap; // FixedJointComponent
		}

		if (entity.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent" << YAML::BeginMap; // BoxColliderComponent

			const auto& boxColliderComponent = entity.GetComponent<BoxColliderComponent>();
			VX_SERIALIZE_PROPERTY(HalfSize, boxColliderComponent.HalfSize, out);
			VX_SERIALIZE_PROPERTY(Offset, boxColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, boxColliderComponent.IsTrigger, out);

			if (AssetManager::IsHandleValid(boxColliderComponent.Material))
			{
				out << YAML::Key << "PhysicsMaterial" << YAML::BeginMap; // PhysicsMaterial

				SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(boxColliderComponent.Material);
				VX_SERIALIZE_PROPERTY(StaticFriction, physicsMaterial->StaticFriction, out);
				VX_SERIALIZE_PROPERTY(DynamicFriction, physicsMaterial->DynamicFriction, out);
				VX_SERIALIZE_PROPERTY(Bounciness, physicsMaterial->Bounciness, out);
				VX_SERIALIZE_PROPERTY(FrictionCombineMode, Utils::CombineModeToString(physicsMaterial->FrictionCombineMode), out);
				VX_SERIALIZE_PROPERTY(BouncinessCombineMode, Utils::CombineModeToString(physicsMaterial->BouncinessCombineMode), out);
				
				out << YAML::EndMap; // PhysicsMaterial
			}

			out << YAML::EndMap; // BoxColliderComponent
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent" << YAML::BeginMap; // SphereColliderComponent

			const auto& sphereColliderComponent = entity.GetComponent<SphereColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, sphereColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Offset, sphereColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, sphereColliderComponent.IsTrigger, out);

			if (AssetManager::IsHandleValid(sphereColliderComponent.Material))
			{
				out << YAML::Key << "PhysicsMaterial" << YAML::BeginMap; // PhysicsMaterial

				SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(sphereColliderComponent.Material);
				VX_SERIALIZE_PROPERTY(StaticFriction, physicsMaterial->StaticFriction, out);
				VX_SERIALIZE_PROPERTY(DynamicFriction, physicsMaterial->DynamicFriction, out);
				VX_SERIALIZE_PROPERTY(Bounciness, physicsMaterial->Bounciness, out);
				VX_SERIALIZE_PROPERTY(FrictionCombineMode, Utils::CombineModeToString(physicsMaterial->FrictionCombineMode), out);
				VX_SERIALIZE_PROPERTY(BouncinessCombineMode, Utils::CombineModeToString(physicsMaterial->BouncinessCombineMode), out);

				out << YAML::EndMap; // PhysicsMaterial
			}

			out << YAML::EndMap; // SphereColliderComponent
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			out << YAML::Key << "CapsuleColliderComponent" << YAML::BeginMap; // CapsuleColliderComponent

			const auto& capsuleColliderComponent = entity.GetComponent<CapsuleColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, capsuleColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Height, capsuleColliderComponent.Height, out);
			VX_SERIALIZE_PROPERTY(Offset, capsuleColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, capsuleColliderComponent.IsTrigger, out);

			if (AssetManager::IsHandleValid(capsuleColliderComponent.Material))
			{
				out << YAML::Key << "PhysicsMaterial" << YAML::BeginMap; // PhysicsMaterial

				SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(capsuleColliderComponent.Material);
				VX_SERIALIZE_PROPERTY(StaticFriction, physicsMaterial->StaticFriction, out);
				VX_SERIALIZE_PROPERTY(DynamicFriction, physicsMaterial->DynamicFriction, out);
				VX_SERIALIZE_PROPERTY(Bounciness, physicsMaterial->Bounciness, out);
				VX_SERIALIZE_PROPERTY(FrictionCombineMode, Utils::CombineModeToString(physicsMaterial->FrictionCombineMode), out);
				VX_SERIALIZE_PROPERTY(BouncinessCombineMode, Utils::CombineModeToString(physicsMaterial->BouncinessCombineMode), out);

				out << YAML::EndMap; // PhysicsMaterial
			}

			out << YAML::EndMap; // CapsuleColliderComponent
		}

		if (entity.HasComponent<MeshColliderComponent>())
		{
			// TODO
		}

		if (entity.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent" << YAML::BeginMap; // Rigidbody2DComponent

			const auto& rb2dComponent = entity.GetComponent<RigidBody2DComponent>();
			VX_SERIALIZE_PROPERTY(BodyType, Utils::RigidBody2DBodyTypeToString(rb2dComponent.Type), out);
			VX_SERIALIZE_PROPERTY(Velocity, rb2dComponent.Velocity, out);
			VX_SERIALIZE_PROPERTY(Drag, rb2dComponent.Drag, out);
			VX_SERIALIZE_PROPERTY(AngularVelocity, rb2dComponent.AngularVelocity, out);
			VX_SERIALIZE_PROPERTY(AngularDrag, rb2dComponent.AngularDrag, out);
			VX_SERIALIZE_PROPERTY(GravityScale, rb2dComponent.GravityScale, out);
			VX_SERIALIZE_PROPERTY(FreezeRotation, rb2dComponent.FixedRotation, out);

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap; // BoxCollider2DComponent

			const auto& bc2dComponent = entity.GetComponent<BoxCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, bc2dComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Size, bc2dComponent.Size, out);
			VX_SERIALIZE_PROPERTY(Density, bc2dComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, bc2dComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, bc2dComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, bc2dComponent.RestitutionThreshold, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, bc2dComponent.IsTrigger, out);

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent" << YAML::BeginMap; // CircleCollider2DComponent

			const auto& cc2dComponent = entity.GetComponent<CircleCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, cc2dComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Radius, cc2dComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Density, cc2dComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, cc2dComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, cc2dComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, cc2dComponent.RestitutionThreshold, out);

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		if (entity.HasComponent<NavMeshAgentComponent>())
		{
			out << YAML::Key << "NavMeshAgentComponent" << YAML::BeginMap; // NavMeshAgentComponent

			const auto& navMeshComponent = entity.GetComponent<NavMeshAgentComponent>();

			out << YAML::EndMap; // NavMeshAgentComponent
		}

		if (entity.HasComponent<ScriptComponent>() && !entity.GetComponent<ScriptComponent>().ClassName.empty())
		{
			const auto& scriptComponent = entity.GetComponent<ScriptComponent>();

			// Script Class Fields
			if (ScriptEngine::EntityClassExists(scriptComponent.ClassName))
			{
				out << YAML::Key << "ScriptComponent" << YAML::BeginMap; // ScriptComponent
				VX_SERIALIZE_PROPERTY(ClassName, scriptComponent.ClassName, out);

				SharedReference<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
				const auto& fields = entityClass->GetFields();

				if (fields.size() > 0)
				{
					const ScriptFieldMap& entityScriptFields = ScriptEngine::GetScriptFieldMap(entity);

					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;

					for (const auto& [name, field] : fields)
					{
						if (entityScriptFields.find(name) == entityScriptFields.end())
							continue;

						out << YAML::BeginMap; // ScriptFields

						VX_SERIALIZE_PROPERTY(Name, name, out);
						VX_SERIALIZE_PROPERTY(Type, ScriptUtils::ScriptFieldTypeToString(field.Type), out);
						out << YAML::Key << "Data" << YAML::Value;

						const ScriptFieldInstance& scriptField = entityScriptFields.at(name);
						
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
							WRITE_SCRIPT_FIELD(AssetHandle, UUID)
						}

						out << YAML::EndMap; // ScriptFields
					}

					out << YAML::EndSeq;
				}
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::DeserializeEntities(YAML::Node& entitiesNode, SharedReference<Scene>& scene)
	{
		SharedReference<EditorAssetManager> editorAssetManager = Project::GetEditorAssetManager();

		for (auto entity : entitiesNode)
		{
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			bool isActive = true;
			if (entity["Active"])
				VX_DESERIALIZE_PROPERTY(Active, bool, isActive, entity);

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

				if (skyboxComponent["Skybox"])
				{
					AssetHandle environmentHandle = skyboxComponent["Skybox"].as<uint64_t>();
					if (AssetManager::IsHandleValid(environmentHandle))
					{
						skybox.Skybox = environmentHandle;
					}
				}

				if (skyboxComponent["Rotation"])
					skybox.Rotation = skyboxComponent["Rotation"].as<float>();

				if (skyboxComponent["Intensity"])
					skybox.Intensity = skyboxComponent["Intensity"].as<float>();
			}

			auto lightSourceComponent = entity["LightSourceComponent"];
			if (lightSourceComponent)
			{
				auto& lightComponent = deserializedEntity.AddComponent<LightSourceComponent>();
				
				if (lightSourceComponent["Visible"])
					lightComponent.Visible = lightSourceComponent["Visible"].as<bool>();

				lightComponent.Type = Utils::LightTypeFromString(lightSourceComponent["LightType"].as<std::string>());

				if (lightSourceComponent["Radiance"])
					lightComponent.Radiance = lightSourceComponent["Radiance"].as<Math::vec3>();

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
							lightComponent.Cutoff = lightSourceComponent["CutOff"].as<float>();
						if (lightSourceComponent["OuterCutOff"])
							lightComponent.OuterCutoff = lightSourceComponent["OuterCutOff"].as<float>();

						break;
					}
				}

				if (lightSourceComponent["Intensity"])
					lightComponent.Intensity = lightSourceComponent["Intensity"].as<float>();

				if (lightSourceComponent["ShadowBias"])
					lightComponent.ShadowBias = lightSourceComponent["ShadowBias"].as<float>();

				if (lightSourceComponent["CastShadows"])
					lightComponent.CastShadows = lightSourceComponent["CastShadows"].as<bool>();

				if (lightSourceComponent["SoftShadows"])
					lightComponent.SoftShadows = lightSourceComponent["SoftShadows"].as<bool>();
			}

			auto meshComponent = entity["MeshRendererComponent"];
			if (meshComponent)
			{
				if (meshComponent["MeshHandle"])
				{
					auto& meshRendererComponent = deserializedEntity.AddComponent<MeshRendererComponent>();
					AssetHandle meshHandle = meshComponent["MeshHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(meshHandle))
					{
						meshRendererComponent.Mesh = meshHandle;

						// Move this to asset serializer
						/*MeshImportOptions importOptions = MeshImportOptions();
						if (meshComponent["MeshImportOptions"])
						{
							auto modelImportOptions = meshComponent["ModelImportOptions"];
							importOptions.MeshTransformation.Translation = modelImportOptions["Translation"].as<Math::vec3>();
							importOptions.MeshTransformation.SetRotationEuler(modelImportOptions["Rotation"].as<Math::vec3>());
							importOptions.MeshTransformation.Scale = modelImportOptions["Scale"].as<Math::vec3>();
						}*/

						if (meshComponent["Visible"])
							meshRendererComponent.Visible = meshComponent["Visible"].as<bool>();
					}
				}
			}

			auto staticMeshComponent = entity["StaticMeshRendererComponent"];
			if (staticMeshComponent)
			{
				auto& staticMeshRendererComponent = deserializedEntity.AddComponent<StaticMeshRendererComponent>();
				staticMeshRendererComponent.Type = Utils::MeshTypeFromString(staticMeshComponent["MeshType"].as<std::string>());

				if (staticMeshRendererComponent.Type != MeshType::Custom)
				{
					DefaultMeshes::StaticMeshes defaultMesh = (DefaultMeshes::StaticMeshes)staticMeshRendererComponent.Type;
					staticMeshRendererComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(defaultMesh);
				}
				else
				{
					if (staticMeshComponent["MeshHandle"])
					{
						AssetHandle staticMeshHandle = staticMeshComponent["MeshHandle"].as<uint64_t>();

						if (AssetManager::IsHandleValid(staticMeshHandle))
						{
							staticMeshRendererComponent.StaticMesh = staticMeshHandle;

							// Move this to asset serializer
							/*MeshImportOptions importOptions = MeshImportOptions();
							if (staticMeshComponent["MeshImportOptions"])
							{
								auto modelImportOptions = staticMeshComponent["MeshImportOptions"];
								importOptions.MeshTransformation.Translation = modelImportOptions["Translation"].as<Math::vec3>();
								importOptions.MeshTransformation.SetRotationEuler(modelImportOptions["Rotation"].as<Math::vec3>());
								importOptions.MeshTransformation.Scale = modelImportOptions["Scale"].as<Math::vec3>();
							}*/

							if (staticMeshComponent["Visible"])
								staticMeshRendererComponent.Visible = staticMeshComponent["Visible"].as<bool>();
						}
					}
				}

				// Load materials
				if (AssetManager::IsHandleValid(staticMeshRendererComponent.StaticMesh))
				{
					// Do this in Asset Serializer
					auto submeshesData = staticMeshComponent["Submeshes"];
					if (submeshesData)
					{
						SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRendererComponent.StaticMesh);

						if (staticMesh)
						{
							SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

							materialTable->Clear();

							uint32_t submeshIndex = 0;
							for (auto submeshData : submeshesData)
							{
								if (!submeshData["MaterialHandle"])
								{
									submeshIndex++;
									continue;
								}

								AssetHandle materialHandle = submeshData["MaterialHandle"].as<uint64_t>();
								if (!AssetManager::IsHandleValid(materialHandle))
								{
									submeshIndex++;
									continue;
								}
							
								materialTable->SetMaterial(submeshIndex, materialHandle);
							}

							if (materialTable->Empty())
							{
								staticMesh->LoadMaterialTable(materialTable);
							}
						}
					}
				}
			}

			auto spriteComponent = entity["SpriteRendererComponent"];
			if (spriteComponent)
			{
				auto& spriteRendererComponent = deserializedEntity.AddComponent<SpriteRendererComponent>();
				spriteRendererComponent.SpriteColor = spriteComponent["Color"].as<Math::vec4>();

				if (spriteComponent["Visible"])
					spriteRendererComponent.Visible = spriteComponent["Visible"].as<bool>();

				if (spriteComponent["TextureHandle"])
				{
					AssetHandle assetHandle = spriteComponent["TextureHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(assetHandle))
					{
						spriteRendererComponent.Texture = assetHandle;
					}

					if (spriteComponent["TextureScale"])
						spriteRendererComponent.TextureUV = spriteComponent["TextureScale"].as<Math::vec2>();
				}
			}

			auto circleComponent = entity["CircleRendererComponent"];
			if (circleComponent)
			{
				auto& circleRendererComponent = deserializedEntity.AddComponent<CircleRendererComponent>();

				if (circleComponent["Visible"])
					circleRendererComponent.Visible = circleComponent["Visible"].as<bool>();

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
				if (particleEmitterComponent["GenerateRandomColors"])
					emitterProperties.GenerateRandomColors = particleEmitterComponent["GenerateRandomColors"].as<bool>();

				particleEmitter.Emitter = ParticleEmitter::Create(emitterProperties);
			}

			auto textMeshComponent = entity["TextMeshComponent"];
			if (textMeshComponent)
			{
				auto& tmc = deserializedEntity.AddComponent<TextMeshComponent>();

				if (textMeshComponent["Visible"])
					tmc.Visible = textMeshComponent["Visible"].as<bool>();

				if (textMeshComponent["FontHandle"])
				{
					AssetHandle fontHandle = textMeshComponent["FontHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(fontHandle))
					{
						tmc.FontAsset = fontHandle;
					}
				}

				tmc.Color = textMeshComponent["Color"].as<Math::vec4>();
				if (textMeshComponent["BgColor"])
					tmc.BgColor = textMeshComponent["BgColor"].as<Math::vec4>();
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
					VX_CONSOLE_LOG_WARN("Trying to add Animation Component without Mesh Renderer Component!");
					return;
				}

				// TODO fix animations to take in mesh asset handle
				/*auto& animation = deserializedEntity.AddComponent<AnimationComponent>();
				SharedRef<Mesh> mesh = deserializedEntity.GetComponent<MeshRendererComponent>().Mesh;
				std::string filepath = mesh->GetPath();
				animation.Animation = Animation::Create(filepath, mesh);*/
			}

			auto animatorComponent = entity["AnimatorComponent"];
			if (animatorComponent)
			{
				if (!deserializedEntity.HasComponent<AnimationComponent>())
				{
					VX_CONSOLE_LOG_WARN("Trying to add Animator Component without Animation Component!");
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
					std::string audioSourcePath = audioSourceComponent["AudioSourcePath"].as<std::string>();
					if (FileSystem::Exists(audioSourcePath))
					{
						asc.Source = AudioSource::Create(audioSourcePath);
					}
				}

				auto soundProps = audioSourceComponent["SoundSettings"];

				if (soundProps)
				{
					auto& deviceProperties = asc.Source->GetProperties();
					if (soundProps["Position"])
						deviceProperties.Position = soundProps["Position"].as<Math::vec3>();
					if (soundProps["Direction"])
						deviceProperties.Direction = soundProps["Direction"].as<Math::vec3>();
					if (soundProps["Velocity"])
						deviceProperties.Velocity = soundProps["Velocity"].as<Math::vec3>();

					if (soundProps["Cone"])
					{
						auto cone = soundProps["Cone"];
						deviceProperties.Cone.InnerAngle = cone["InnerAngle"].as<float>();
						deviceProperties.Cone.OuterAngle = cone["OuterAngle"].as<float>();
						deviceProperties.Cone.OuterGain = cone["OuterGain"].as<float>();
					}

					if (soundProps["MinGain"])
						deviceProperties.MinGain = soundProps["MinGain"].as<float>();
					if (soundProps["MaxGain"])
						deviceProperties.MaxGain = soundProps["MaxGain"].as<float>();

					if (soundProps["AttenuationModel"])
						deviceProperties.AttenuationModel = Utils::AttenuationModelTypeFromString(soundProps["AttenuationModel"].as<std::string>());
					if (soundProps["Falloff"])
						deviceProperties.Falloff = soundProps["Falloff"].as<float>();

					if (soundProps["MinDistance"])
						deviceProperties.MinDistance = soundProps["MinDistance"].as<float>();
					if (soundProps["MaxDistance"])
						deviceProperties.MaxDistance = soundProps["MaxDistance"].as<float>();

					if (soundProps["Pitch"])
						deviceProperties.Pitch = soundProps["Pitch"].as<float>();
					if (soundProps["DopplerFactor"])
						deviceProperties.DopplerFactor = soundProps["DopplerFactor"].as<float>();
					if (soundProps["Volume"])
						deviceProperties.Volume = soundProps["Volume"].as<float>();

					if (soundProps["PlayOnStart"])
						deviceProperties.PlayOnStart = soundProps["PlayOnStart"].as<bool>();
					if (soundProps["PlayOneShot"])
						deviceProperties.PlayOneShot = soundProps["PlayOneShot"].as<bool>();
					if (soundProps["Spacialized"])
						deviceProperties.Spacialized = soundProps["Spacialized"].as<bool>();
					if (soundProps["Loop"])
						deviceProperties.Loop = soundProps["Loop"].as<bool>();

					asc.Source->SetProperties(deviceProperties);
				}
			}

			auto rigidbodyComponent = entity["RigidbodyComponent"];
			if (rigidbodyComponent)
			{
				auto& rigidbody = deserializedEntity.AddComponent<RigidBodyComponent>();

				rigidbody.Type = Utils::RigidBodyTypeFromString(rigidbodyComponent["BodyType"].as<std::string>());
				if (rigidbodyComponent["Mass"])
					rigidbody.Mass = rigidbodyComponent["Mass"].as<float>();
				if (rigidbodyComponent["AngularDrag"])
					rigidbody.AngularDrag = rigidbodyComponent["AngularDrag"].as<float>();
				if (rigidbodyComponent["MaxAngularVelocity"])
					rigidbody.MaxAngularVelocity = rigidbodyComponent["MaxAngularVelocity"].as<float>();
				if (rigidbodyComponent["AngularVelocity"])
					rigidbody.AngularVelocity = rigidbodyComponent["AngularVelocity"].as<Math::vec3>();
				if (rigidbodyComponent["DisableGravity"])
					rigidbody.DisableGravity = rigidbodyComponent["DisableGravity"].as<bool>();
				if (rigidbodyComponent["IsKinematic"])
					rigidbody.IsKinematic = rigidbodyComponent["IsKinematic"].as<bool>();
				if (rigidbodyComponent["LinearDrag"])
					rigidbody.LinearDrag = rigidbodyComponent["LinearDrag"].as<float>();
				if (rigidbodyComponent["MaxLinearVelocity"])
					rigidbody.MaxLinearVelocity = rigidbodyComponent["MaxLinearVelocity"].as<float>();
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

			auto fixedJointComponent = entity["FixedJointComponent"];
			if (fixedJointComponent)
			{
				auto& fixedJoint = deserializedEntity.AddComponent<FixedJointComponent>();

				VX_DESERIALIZE_PROPERTY(ConnectedEntity, uint64_t, fixedJoint.ConnectedEntity, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(BreakForce, float, fixedJoint.BreakForce, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(BreakTorque, float, fixedJoint.BreakTorque, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(EnableCollision, bool, fixedJoint.EnableCollision, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(EnablePreProcessing, bool, fixedJoint.EnablePreProcessing, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(IsBreakable, bool, fixedJoint.IsBreakable, fixedJointComponent);
			}

			auto boxColliderComponent = entity["BoxColliderComponent"];
			if (boxColliderComponent)
			{
				auto& boxCollider = deserializedEntity.AddComponent<BoxColliderComponent>();

				boxCollider.HalfSize = boxColliderComponent["HalfSize"].as<glm::vec3>();
				boxCollider.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
				if (boxColliderComponent["IsTrigger"])
					boxCollider.IsTrigger = boxColliderComponent["IsTrigger"].as<bool>();

				auto physicsMaterialData = boxColliderComponent["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					AssetHandle materialHandle = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>();
					SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(materialHandle);

					physicsMaterial->StaticFriction = physicsMaterialData["StaticFriction"].as<float>();
					physicsMaterial->DynamicFriction = physicsMaterialData["DynamicFriction"].as<float>();
					physicsMaterial->Bounciness = physicsMaterialData["Bounciness"].as<float>();

					if (physicsMaterialData["FrictionCombineMode"])
						physicsMaterial->FrictionCombineMode = Utils::CombineModeFromString(physicsMaterialData["FrictionCombineMode"].as<std::string>());
					if (physicsMaterialData["BouncinessCombineMode"])
						physicsMaterial->BouncinessCombineMode = Utils::CombineModeFromString(physicsMaterialData["BouncinessCombineMode"].as<std::string>());
				}
			}

			auto sphereColliderComponent = entity["SphereColliderComponent"];
			if (sphereColliderComponent)
			{
				auto& sphereCollider = deserializedEntity.AddComponent<SphereColliderComponent>();

				sphereCollider.Radius = sphereColliderComponent["Radius"].as<float>();
				sphereCollider.Offset = sphereColliderComponent["Offset"].as<Math::vec3>();
				sphereCollider.IsTrigger = sphereColliderComponent["IsTrigger"].as<bool>();

				auto physicsMaterialData = boxColliderComponent["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					AssetHandle materialHandle = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>();
					SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(materialHandle);

					physicsMaterial->StaticFriction = physicsMaterialData["StaticFriction"].as<float>();
					physicsMaterial->DynamicFriction = physicsMaterialData["DynamicFriction"].as<float>();
					physicsMaterial->Bounciness = physicsMaterialData["Bounciness"].as<float>();

					if (physicsMaterialData["FrictionCombineMode"])
						physicsMaterial->FrictionCombineMode = Utils::CombineModeFromString(physicsMaterialData["FrictionCombineMode"].as<std::string>());
					if (physicsMaterialData["BouncinessCombineMode"])
						physicsMaterial->BouncinessCombineMode = Utils::CombineModeFromString(physicsMaterialData["BouncinessCombineMode"].as<std::string>());
				}
			}

			auto capsuleColliderComponent = entity["CapsuleColliderComponent"];
			if (capsuleColliderComponent)
			{
				auto& capsuleCollider = deserializedEntity.AddComponent<CapsuleColliderComponent>();

				capsuleCollider.Radius = capsuleColliderComponent["Radius"].as<float>();
				capsuleCollider.Height = capsuleColliderComponent["Height"].as<float>();
				capsuleCollider.Offset = capsuleColliderComponent["Offset"].as<Math::vec3>();
				capsuleCollider.IsTrigger = capsuleColliderComponent["IsTrigger"].as<bool>();

				auto physicsMaterialData = boxColliderComponent["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					AssetHandle materialHandle = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>();
					SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(materialHandle);

					physicsMaterial->StaticFriction = physicsMaterialData["StaticFriction"].as<float>();
					physicsMaterial->DynamicFriction = physicsMaterialData["DynamicFriction"].as<float>();
					physicsMaterial->Bounciness = physicsMaterialData["Bounciness"].as<float>();

					if (physicsMaterialData["FrictionCombineMode"])
						physicsMaterial->FrictionCombineMode = Utils::CombineModeFromString(physicsMaterialData["FrictionCombineMode"].as<std::string>());
					if (physicsMaterialData["BouncinessCombineMode"])
						physicsMaterial->BouncinessCombineMode = Utils::CombineModeFromString(physicsMaterialData["BouncinessCombineMode"].as<std::string>());
				}
			}

			auto meshColliderComponent = entity["MeshColliderComponent"];
			if (meshColliderComponent)
			{
				auto& meshCollider = deserializedEntity.AddComponent<MeshColliderComponent>();

				// TODO
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
				if (rigidbody2DComponent["AngularVelocity"])
					rb2d.AngularVelocity = rigidbody2DComponent["AngularVelocity"].as<float>();
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

				if (ScriptEngine::EntityClassExists(sc.ClassName))
				{
					auto scriptFields = scriptComponent["ScriptFields"];

					if (scriptFields)
					{
						SharedReference<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);

						if (entityClass)
						{
							const auto& fields = entityClass->GetFields();
							ScriptFieldMap& entityScriptFields = ScriptEngine::GetMutableScriptFieldMap(deserializedEntity);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();

								ScriptFieldInstance& fieldInstance = entityScriptFields[name];

								if (fields.find(name) == fields.end())
								{
									VX_CONSOLE_LOG_WARN("Script Field '{}' was not found in Field Map!", name);
									continue;
								}

								fieldInstance.Field = fields.at(name);

								std::string typeString = scriptField["Type"].as<std::string>();
								ScriptFieldType type = ScriptUtils::StringToScriptFieldType(typeString);

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
									READ_SCRIPT_FIELD(AssetHandle, UUID)
								}
							}
						}
					}	
				}
			}
		}
	}

}
