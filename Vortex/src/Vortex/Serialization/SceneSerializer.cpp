#include "vxpch.h"
#include "SceneSerializer.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Asset/AssetManager.h"
#include "Vortex/Asset/AssetImporter.h"

#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Animation/Animation.h"
#include "Vortex/Animation/Animator.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"
#include "Vortex/Renderer/Font/Font.h"

#include "Vortex/Physics/3D/PhysicsMaterial.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ScriptClass.h"

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

	SceneSerializer::SceneSerializer(const SharedReference<Scene>& scene)
		: m_Scene(scene) { }

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		size_t extensionPos = filepath.find(".");
		size_t lastSlashPos = filepath.find_last_of("/\\") + 1;
		size_t length = filepath.length();
		const bool invalidFile = length == 0;

		const std::string sceneName = invalidFile ? "Untitled" : filepath.substr(lastSlashPos, extensionPos - lastSlashPos);

		YAML::Emitter out;
		out << YAML::BeginMap; // Scene
		
		out << YAML::Key << "Scene" << YAML::Value << sceneName;
		out << YAML::Key << "Actors" << YAML::Value;
		out << YAML::BeginSeq; // Actors

		m_Scene->m_Registry.each([&](auto actorID)
		{
			Actor actor = { actorID, m_Scene.Raw() };

			if (!actor)
				return;

			SerializeActor(out, actor);
		});

		out << YAML::EndSeq; // Actors
		out << YAML::EndMap; // Scene

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
		{
			return false;
		}

		const std::string sceneName = data["Scene"].as<std::string>();
		VX_CONSOLE_LOG_INFO("[Scene Serializer] Deserializing Scene '{}'", sceneName);

		// (NOTE):                                                  for backwards compatability
		const YAML::Node actors = data["Actors"] ? data["Actors"] : data["Entities"];
		if (!actors)
		{
			return false;
		}

		DeserializeActors(actors, m_Scene);

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

	void SceneSerializer::SerializeActor(YAML::Emitter& out, Actor actor)
	{
		VX_CORE_ASSERT(actor.HasComponent<IDComponent>(), "Actor does not have a universally unique identifier!");

		const std::filesystem::path projectAssetDirectory = Project::GetAssetDirectory();

		out << YAML::BeginMap; // Actor

		VX_SERIALIZE_PROPERTY(Actor, actor.GetUUID(), out);
		VX_SERIALIZE_PROPERTY(Active, actor.IsActive(), out);

		if (actor.HasComponent<HierarchyComponent>())
		{
			const HierarchyComponent& hierarchyComponent = actor.GetComponent<HierarchyComponent>();
			VX_SERIALIZE_PROPERTY(Parent, hierarchyComponent.ParentUUID, out);

			out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;

			for (const UUID& child : hierarchyComponent.Children)
			{
				out << YAML::BeginMap;
				VX_SERIALIZE_PROPERTY(Handle, child, out);
				out << YAML::EndMap;
			}

			out << YAML::EndSeq;
		}

		if (actor.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // TagComponent

			const TagComponent& tagComponent = actor.GetComponent<TagComponent>();

			VX_SERIALIZE_PROPERTY(Tag, tagComponent.Tag, out);
			VX_SERIALIZE_PROPERTY(Marker, tagComponent.Marker, out);

			out << YAML::EndMap; // TagComponent
		}

		if (actor.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent

			const TransformComponent& transformComponent = actor.GetComponent<TransformComponent>();

			VX_SERIALIZE_PROPERTY(Translation, transformComponent.Translation, out);
			VX_SERIALIZE_PROPERTY(Rotation, transformComponent.GetRotationEuler(), out);
			VX_SERIALIZE_PROPERTY(Scale, transformComponent.Scale, out);

			out << YAML::EndMap; // TransformComponent
		}

		if (actor.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent" << YAML::BeginMap; // CameraComponent

			const CameraComponent& cameraComponent = actor.GetComponent<CameraComponent>();
			const SceneCamera& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;

			out << YAML::BeginMap; // Camera
			VX_SERIALIZE_PROPERTY(ProjectionType, (int)camera.GetProjectionType(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveFOV, camera.GetPerspectiveFOV(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveNear, camera.GetPerspectiveNearClip(), out);
			VX_SERIALIZE_PROPERTY(PerspectiveFar, camera.GetPerspectiveFarClip(), out);
			VX_SERIALIZE_PROPERTY(OrthographicSize, camera.GetOrthographicSize(), out);
			VX_SERIALIZE_PROPERTY(OrthographicNear, camera.GetOrthographicNearClip(), out);
			VX_SERIALIZE_PROPERTY(OrthographicFar, camera.GetOrthographicFarClip(), out);
			out << YAML::EndMap; // Camera

			VX_SERIALIZE_PROPERTY(ClearColor, cameraComponent.ClearColor, out);
			VX_SERIALIZE_PROPERTY(Primary, cameraComponent.Primary, out);
			VX_SERIALIZE_PROPERTY(FixedAspectRatio, cameraComponent.FixedAspectRatio, out);
			
			VX_SERIALIZE_PROPERTY(PostProcessingEnabled, cameraComponent.PostProcessing.Enabled, out);

			if (cameraComponent.PostProcessing.Enabled)
			{
				out << YAML::Key << "PostProcessing" << YAML::Value << YAML::BeginMap; // PostProcessing
				
				{
					out << YAML::Key << "Bloom" << YAML::Value << YAML::BeginMap; // Bloom
					VX_SERIALIZE_PROPERTY(Threshold, cameraComponent.PostProcessing.Bloom.Threshold, out);
					VX_SERIALIZE_PROPERTY(Knee, cameraComponent.PostProcessing.Bloom.Knee, out);
					VX_SERIALIZE_PROPERTY(Intensity, cameraComponent.PostProcessing.Bloom.Intensity, out);
					VX_SERIALIZE_PROPERTY(Enabled, cameraComponent.PostProcessing.Bloom.Enabled, out);
					out << YAML::EndMap; // Bloom
				}

				out << YAML::EndMap; // PostProcessing
			}

			out << YAML::EndMap; // CameraComponent
		}

		if (actor.HasComponent<SkyboxComponent>())
		{
			const SkyboxComponent& skyboxComponent = actor.GetComponent<SkyboxComponent>();
			AssetHandle environmentHandle = skyboxComponent.Skybox;
			
			out << YAML::Key << "SkyboxComponent" << YAML::BeginMap; // SkyboxComponent

			VX_SERIALIZE_PROPERTY(Skybox, environmentHandle, out);
			VX_SERIALIZE_PROPERTY(Rotation, skyboxComponent.Rotation, out);
			VX_SERIALIZE_PROPERTY(Intensity, skyboxComponent.Intensity, out);

			out << YAML::EndMap; // SkyboxComponent
		}

		if (actor.HasComponent<LightSourceComponent>())
		{
			out << YAML::Key << "LightSourceComponent";
			out << YAML::BeginMap; // LightSourceComponent

			const LightSourceComponent& lightSourceComponent = actor.GetComponent<LightSourceComponent>();

			VX_SERIALIZE_PROPERTY(Visible, lightSourceComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(LightType, Utils::LightTypeToString(lightSourceComponent.Type), out);

			VX_SERIALIZE_PROPERTY(Radiance, lightSourceComponent.Radiance, out);

			switch (lightSourceComponent.Type)
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
					VX_SERIALIZE_PROPERTY(CutOff, lightSourceComponent.Cutoff,  out);
					VX_SERIALIZE_PROPERTY(OuterCutOff, lightSourceComponent.OuterCutoff, out);

					break;
				}
			}

			VX_SERIALIZE_PROPERTY(Intensity, lightSourceComponent.Intensity, out);
			VX_SERIALIZE_PROPERTY(ShadowBias, lightSourceComponent.ShadowBias, out);
			VX_SERIALIZE_PROPERTY(CastShadows, lightSourceComponent.CastShadows, out);
			VX_SERIALIZE_PROPERTY(SoftShadows, lightSourceComponent.SoftShadows, out);

			out << YAML::EndMap; // LightSourceComponent
		}

		if (actor.HasComponent<MeshRendererComponent>())
		{
			const MeshRendererComponent& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
			AssetHandle meshHandle = meshRendererComponent.Mesh;

			if (AssetManager::IsHandleValid(meshHandle))
			{
				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
				if (mesh)
				{
					out << YAML::Key << "MeshRendererComponent" << YAML::Value << YAML::BeginMap; // MeshRendererComponent

					VX_SERIALIZE_PROPERTY(MeshHandle, meshHandle, out);
					VX_SERIALIZE_PROPERTY(Visible, meshRendererComponent.Visible, out);
					VX_SERIALIZE_PROPERTY(CastShadows, meshRendererComponent.CastShadows, out);

					if (MeshImportOptions importOptions = mesh->GetImportOptions(); importOptions != MeshImportOptions{})
					{
						out << YAML::Key << "MeshImportOptions" << YAML::Value << YAML::BeginMap; // MeshImportOptions

						VX_SERIALIZE_PROPERTY(Translation, importOptions.MeshTransformation.Translation, out);
						VX_SERIALIZE_PROPERTY(Rotation, importOptions.MeshTransformation.GetRotationEuler(), out);
						VX_SERIALIZE_PROPERTY(Scale, importOptions.MeshTransformation.Scale, out);

						out << YAML::EndMap; // MeshImportOptions
					}

					{
						const Submesh& submesh = mesh->GetSubmesh();

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

		if (actor.HasComponent<StaticMeshRendererComponent>())
		{
			const StaticMeshRendererComponent& staticMeshRendererComponent = actor.GetComponent<StaticMeshRendererComponent>();
			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			
			if (AssetManager::IsHandleValid(staticMeshHandle))
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
				
				if (staticMesh)
				{
					out << YAML::Key << "StaticMeshRendererComponent" << YAML::Value << YAML::BeginMap; // StaticMeshRendererComponent

					VX_SERIALIZE_PROPERTY(MeshHandle, staticMeshHandle, out);
					VX_SERIALIZE_PROPERTY(MeshType, Utils::MeshTypeToString(staticMeshRendererComponent.Type), out);
					VX_SERIALIZE_PROPERTY(Visible, staticMeshRendererComponent.Visible, out);
					VX_SERIALIZE_PROPERTY(CastShadows, staticMeshRendererComponent.CastShadows, out);

					if (const MeshImportOptions& importOptions = staticMesh->GetImportOptions(); importOptions != MeshImportOptions{})
					{
						out << YAML::Key << "MeshImportOptions" << YAML::Value << YAML::BeginMap; // MeshImportOptions

						VX_SERIALIZE_PROPERTY(Translation, importOptions.MeshTransformation.Translation, out);
						VX_SERIALIZE_PROPERTY(Rotation, importOptions.MeshTransformation.GetRotationEuler(), out);
						VX_SERIALIZE_PROPERTY(Scale, importOptions.MeshTransformation.Scale, out);

						out << YAML::EndMap; // MeshImportOptions
					}

					out << YAML::Key << "Submeshes" << YAML::Value << YAML::BeginSeq;
					{
						const std::unordered_map<uint32_t, StaticSubmesh>& submeshes = staticMesh->GetSubmeshes();

						for (const auto& [submeshIndex, submesh] : submeshes)
						{
							SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;
							AssetHandle materialHandle = materialTable->GetMaterial(submeshIndex);
							if (!AssetManager::IsHandleValid(materialHandle))
								continue;

							SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);
							if (!material)
								continue;

							AssetImporter::Serialize(material);

							out << YAML::BeginMap; // Submesh
							
							VX_SERIALIZE_PROPERTY(Name, submesh.GetName(), out);
							VX_SERIALIZE_PROPERTY(MaterialHandle, materialHandle, out);

							out << YAML::EndMap; // Submesh
						}
					}
					out << YAML::EndSeq; // Submeshes
					
					out << YAML::EndMap; // StaticMeshRendererComponent
				}
			}
		}

		if (actor.HasComponent<SpriteRendererComponent>())
		{
			const SpriteRendererComponent& spriteRendererComponent = actor.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent" << YAML::Value << YAML::BeginMap; // SpriteRendererComponent

			AssetHandle spriteHandle = spriteRendererComponent.Texture;
			VX_SERIALIZE_PROPERTY(TextureHandle, spriteHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, spriteRendererComponent.Visible, out);

			VX_SERIALIZE_PROPERTY(Color, spriteRendererComponent.SpriteColor, out);
			VX_SERIALIZE_PROPERTY(TextureUV, spriteRendererComponent.TextureUV, out);

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (actor.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent" << YAML::Value << YAML::BeginMap; // CircleRendererComponent

			const CircleRendererComponent& circleRendererComponent = actor.GetComponent<CircleRendererComponent>();

			VX_SERIALIZE_PROPERTY(Visible, circleRendererComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, circleRendererComponent.Color, out);
			VX_SERIALIZE_PROPERTY(Thickness, circleRendererComponent.Thickness, out);
			VX_SERIALIZE_PROPERTY(Fade, circleRendererComponent.Fade, out);

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (actor.HasComponent<ParticleEmitterComponent>())
		{
			const ParticleEmitterComponent& particleEmitterComponent = actor.GetComponent<ParticleEmitterComponent>();
			AssetHandle emitterHandle = particleEmitterComponent.EmitterHandle;

			out << YAML::Key << "ParticleEmitterComponent" << YAML::Value << YAML::BeginMap; // ParticleEmitterComponent

			if (AssetManager::IsHandleValid(emitterHandle))
			{
				SharedReference<ParticleEmitter> particleEmitter = AssetManager::GetAsset<ParticleEmitter>(emitterHandle);

				if (particleEmitter)
				{
					AssetImporter::Serialize(particleEmitter);

					VX_SERIALIZE_PROPERTY(EmitterHandle, particleEmitter->Handle, out);
				}
			}

			VX_SERIALIZE_PROPERTY(IsActive, particleEmitterComponent.IsActive, out);

			out << YAML::EndMap; // ParticleEmitterComponent
		}

		if (actor.HasComponent<TextMeshComponent>())
		{
			const TextMeshComponent& textMeshComponent = actor.GetComponent<TextMeshComponent>();
			AssetHandle fontHandle = textMeshComponent.FontAsset;

			out << YAML::Key << "TextMeshComponent" << YAML::Value << YAML::BeginMap; // TextMeshComponent

			VX_SERIALIZE_PROPERTY(FontHandle, fontHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, textMeshComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, textMeshComponent.Color, out);
			VX_SERIALIZE_PROPERTY(BackgroundColor, textMeshComponent.BackgroundColor, out);
			VX_SERIALIZE_PROPERTY(Kerning, textMeshComponent.Kerning, out);
			VX_SERIALIZE_PROPERTY(LineSpacing, textMeshComponent.LineSpacing, out);
			VX_SERIALIZE_PROPERTY(MaxWidth, textMeshComponent.MaxWidth, out);
			VX_SERIALIZE_PROPERTY(TextHash, textMeshComponent.TextHash, out);
			VX_SERIALIZE_PROPERTY(TextString, textMeshComponent.TextString, out);
			
			VX_SERIALIZE_PROPERTY(DropShadowEnabled, textMeshComponent.DropShadow.Enabled, out);
			
			if (textMeshComponent.DropShadow.Enabled)
			{
				out << YAML::Key << "DropShadow" << YAML::Value << YAML::BeginMap; // DropShadow
				VX_SERIALIZE_PROPERTY(Color, textMeshComponent.DropShadow.Color, out);
				VX_SERIALIZE_PROPERTY(ShadowDistance, textMeshComponent.DropShadow.ShadowDistance, out);
				VX_SERIALIZE_PROPERTY(ShadowScale, textMeshComponent.DropShadow.ShadowScale, out);
				out << YAML::EndMap; // DropShadow
			}

			out << YAML::EndMap; // TextMeshComponent
		}

		if (actor.HasComponent<AnimatorComponent>())
		{
			out << YAML::Key << "AnimatorComponent" << YAML::Value << YAML::BeginMap; // AnimatorComponent

			const AnimatorComponent& animatorComponent = actor.GetComponent<AnimatorComponent>();
			SharedRef<Animator> animator = animatorComponent.Animator;

			out << YAML::EndMap; // AnimatorComponent
		}

		if (actor.HasComponent<AnimationComponent>())
		{
			out << YAML::Key << "AnimationComponent" << YAML::Value << YAML::BeginMap; // AnimationComponent

			const AnimationComponent& animationComponent = actor.GetComponent<AnimationComponent>();
			SharedRef<Animation> animation = animationComponent.Animation;

			VX_SERIALIZE_PROPERTY(AnimationSourcePath, animation->GetPath(), out);

			out << YAML::EndMap; // AnimationComponent
		}

		if (actor.HasComponent<AudioSourceComponent>())
		{
			const AudioSourceComponent& audioSourceComponent = actor.GetComponent<AudioSourceComponent>();

			out << YAML::Key << "AudioSourceComponent" << YAML::Value << YAML::BeginMap; //AudioSourceComponent
			if (AssetManager::IsHandleValid(audioSourceComponent.AudioHandle))
			{
				SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(audioSourceComponent.AudioHandle);
				
				if (audioSource)
				{
					AssetImporter::Serialize(audioSource);

					VX_SERIALIZE_PROPERTY(AudioHandle, audioSourceComponent.AudioHandle, out);
				}
			}

			VX_SERIALIZE_PROPERTY(PlayOnStart, audioSourceComponent.PlayOnStart, out);
			VX_SERIALIZE_PROPERTY(PlayOneShot, audioSourceComponent.PlayOneShot, out);

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (actor.HasComponent<AudioListenerComponent>())
		{
			// TODO
		}

		if (actor.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent" << YAML::BeginMap; // RigidbodyComponent

			const RigidBodyComponent& rigidBodyComponent = actor.GetComponent<RigidBodyComponent>();
			VX_SERIALIZE_PROPERTY(Mass, rigidBodyComponent.Mass, out);
			VX_SERIALIZE_PROPERTY(BodyType, Utils::RigidBodyTypeToString(rigidBodyComponent.Type), out);
			VX_SERIALIZE_PROPERTY(AngularDrag, rigidBodyComponent.AngularDrag, out);
			VX_SERIALIZE_PROPERTY(MaxAngularVelocity, rigidBodyComponent.MaxAngularVelocity, out);
			VX_SERIALIZE_PROPERTY(AngularVelocity, rigidBodyComponent.AngularVelocity, out);
			VX_SERIALIZE_PROPERTY(DisableGravity, rigidBodyComponent.DisableGravity, out);
			VX_SERIALIZE_PROPERTY(IsKinematic, rigidBodyComponent.IsKinematic, out);
			VX_SERIALIZE_PROPERTY(LinearDrag, rigidBodyComponent.LinearDrag, out);
			VX_SERIALIZE_PROPERTY(MaxLinearVelocity, rigidBodyComponent.MaxLinearVelocity, out);
			VX_SERIALIZE_PROPERTY(LinearVelocity, rigidBodyComponent.LinearVelocity, out);
			VX_SERIALIZE_PROPERTY(CollisionDetectionType, Utils::CollisionDetectionTypeToString(rigidBodyComponent.CollisionDetection), out);
			VX_SERIALIZE_PROPERTY(ActorLockFlags, (uint32_t)rigidBodyComponent.LockFlags, out);

			out << YAML::EndMap; // RigidbodyComponent
		}

		if (actor.HasComponent<CharacterControllerComponent>())
		{
			out << YAML::Key << "CharacterControllerComponent" << YAML::BeginMap; // CharacterControllerComponent

			const CharacterControllerComponent& characterControllerComponent = actor.GetComponent<CharacterControllerComponent>();
			VX_SERIALIZE_PROPERTY(NonWalkableMode, Utils::NonWalkableModeToString(characterControllerComponent.NonWalkMode), out);
			VX_SERIALIZE_PROPERTY(CapsuleClimbMode, Utils::CapsuleClimbModeToString(characterControllerComponent.ClimbMode), out);
			VX_SERIALIZE_PROPERTY(DisableGravity, characterControllerComponent.DisableGravity, out);
			VX_SERIALIZE_PROPERTY(LayerID, characterControllerComponent.LayerID, out);
			VX_SERIALIZE_PROPERTY(SlopeLimitDegrees, characterControllerComponent.SlopeLimitDegrees, out);
			VX_SERIALIZE_PROPERTY(StepOffset, characterControllerComponent.StepOffset, out);
			VX_SERIALIZE_PROPERTY(ContactOffset, characterControllerComponent.ContactOffset, out);

			out << YAML::EndMap; // CharacterControllerComponent
		}

		if (actor.HasComponent<FixedJointComponent>())
		{
			out << YAML::Key << "FixedJointComponent" << YAML::BeginMap; // FixedJointComponent

			const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			VX_SERIALIZE_PROPERTY(ConnectedActor, fixedJointComponent.ConnectedActor, out);
			VX_SERIALIZE_PROPERTY(BreakForce, fixedJointComponent.BreakForce, out);
			VX_SERIALIZE_PROPERTY(BreakTorque, fixedJointComponent.BreakTorque, out);
			VX_SERIALIZE_PROPERTY(EnableCollision, fixedJointComponent.EnableCollision, out);
			VX_SERIALIZE_PROPERTY(EnablePreProcessing, fixedJointComponent.EnablePreProcessing, out);
			VX_SERIALIZE_PROPERTY(IsBreakable, fixedJointComponent.IsBreakable, out);

			out << YAML::EndMap; // FixedJointComponent
		}

		auto SerializePhysicsMaterialFn = [&](auto handle) {
			out << YAML::Key << "PhysicsMaterial" << YAML::BeginMap; // PhysicsMaterial

			SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(handle);
			VX_SERIALIZE_PROPERTY(StaticFriction, physicsMaterial->StaticFriction, out);
			VX_SERIALIZE_PROPERTY(DynamicFriction, physicsMaterial->DynamicFriction, out);
			VX_SERIALIZE_PROPERTY(Bounciness, physicsMaterial->Bounciness, out);
			VX_SERIALIZE_PROPERTY(FrictionCombineMode, Utils::CombineModeToString(physicsMaterial->FrictionCombineMode), out);
			VX_SERIALIZE_PROPERTY(BouncinessCombineMode, Utils::CombineModeToString(physicsMaterial->BouncinessCombineMode), out);

			out << YAML::EndMap; // PhysicsMaterial
		};
		
		if (actor.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent" << YAML::BeginMap; // BoxColliderComponent

			const BoxColliderComponent& boxColliderComponent = actor.GetComponent<BoxColliderComponent>();
			VX_SERIALIZE_PROPERTY(HalfSize, boxColliderComponent.HalfSize, out);
			VX_SERIALIZE_PROPERTY(Offset, boxColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, boxColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, boxColliderComponent.Visible, out);

			if (AssetManager::IsHandleValid(boxColliderComponent.Material))
			{
				SerializePhysicsMaterialFn(boxColliderComponent.Material);
			}

			out << YAML::EndMap; // BoxColliderComponent
		}

		if (actor.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent" << YAML::BeginMap; // SphereColliderComponent

			const SphereColliderComponent& sphereColliderComponent = actor.GetComponent<SphereColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, sphereColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Offset, sphereColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, sphereColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, sphereColliderComponent.Visible, out);

			if (AssetManager::IsHandleValid(sphereColliderComponent.Material))
			{
				SerializePhysicsMaterialFn(sphereColliderComponent.Material);
			}

			out << YAML::EndMap; // SphereColliderComponent
		}

		if (actor.HasComponent<CapsuleColliderComponent>())
		{
			out << YAML::Key << "CapsuleColliderComponent" << YAML::BeginMap; // CapsuleColliderComponent

			const CapsuleColliderComponent& capsuleColliderComponent = actor.GetComponent<CapsuleColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, capsuleColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Height, capsuleColliderComponent.Height, out);
			VX_SERIALIZE_PROPERTY(Offset, capsuleColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, capsuleColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, capsuleColliderComponent.Visible, out);

			if (AssetManager::IsHandleValid(capsuleColliderComponent.Material))
			{
				SerializePhysicsMaterialFn(capsuleColliderComponent.Material);
			}

			out << YAML::EndMap; // CapsuleColliderComponent
		}

		if (actor.HasComponent<MeshColliderComponent>())
		{
			// TODO
		}

		if (actor.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent" << YAML::BeginMap; // Rigidbody2DComponent

			const RigidBody2DComponent& rigidBodyComponent = actor.GetComponent<RigidBody2DComponent>();
			VX_SERIALIZE_PROPERTY(BodyType, Utils::RigidBody2DBodyTypeToString(rigidBodyComponent.Type), out);
			VX_SERIALIZE_PROPERTY(Velocity, rigidBodyComponent.Velocity, out);
			VX_SERIALIZE_PROPERTY(Drag, rigidBodyComponent.Drag, out);
			VX_SERIALIZE_PROPERTY(AngularVelocity, rigidBodyComponent.AngularVelocity, out);
			VX_SERIALIZE_PROPERTY(AngularDrag, rigidBodyComponent.AngularDrag, out);
			VX_SERIALIZE_PROPERTY(GravityScale, rigidBodyComponent.GravityScale, out);
			VX_SERIALIZE_PROPERTY(FreezeRotation, rigidBodyComponent.FixedRotation, out);

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (actor.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap; // BoxCollider2DComponent

			const BoxCollider2DComponent& boxColliderComponent = actor.GetComponent<BoxCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, boxColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Size, boxColliderComponent.Size, out);
			VX_SERIALIZE_PROPERTY(Density, boxColliderComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, boxColliderComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, boxColliderComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, boxColliderComponent.RestitutionThreshold, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, boxColliderComponent.IsTrigger, out);

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (actor.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent" << YAML::BeginMap; // CircleCollider2DComponent

			const CircleCollider2DComponent& circleColliderComponent = actor.GetComponent<CircleCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, circleColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Radius, circleColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Density, circleColliderComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, circleColliderComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, circleColliderComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, circleColliderComponent.RestitutionThreshold, out);

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		if (actor.HasComponent<NavMeshAgentComponent>())
		{
			out << YAML::Key << "NavMeshAgentComponent" << YAML::BeginMap; // NavMeshAgentComponent

			const NavMeshAgentComponent& navMeshAgentComponent = actor.GetComponent<NavMeshAgentComponent>();

			out << YAML::EndMap; // NavMeshAgentComponent
		}

		if (actor.HasComponent<ScriptComponent>() && ScriptEngine::IsScriptClassValid(actor))
		{
			const ScriptComponent& scriptComponent = actor.GetComponent<ScriptComponent>();

			// Script Class Fields
			out << YAML::Key << "ScriptComponent" << YAML::BeginMap; // ScriptComponent
			VX_SERIALIZE_PROPERTY(ClassName, scriptComponent.ClassName, out);
			VX_SERIALIZE_PROPERTY(Enabled, scriptComponent.Enabled, out);

			SharedReference<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(scriptComponent.ClassName);
			const std::map<std::string, ScriptField>& scriptFields = scriptClass->GetFields();

			if (scriptFields.size() > 0)
			{
				const ScriptFieldMap& actorScriptFields = ScriptEngine::GetScriptFieldMap(actor);

				out << YAML::Key << "ScriptFields" << YAML::Value;
				out << YAML::BeginSeq;

				for (const auto& [name, field] : scriptFields)
				{
					if (actorScriptFields.find(name) == actorScriptFields.end())
						continue;

					out << YAML::BeginMap; // ScriptFields

					VX_SERIALIZE_PROPERTY(Name, name, out);
					VX_SERIALIZE_PROPERTY(Type, ScriptUtils::ScriptFieldTypeToString(field.Type), out);
					out << YAML::Key << "Data" << YAML::Value;

					const ScriptFieldInstance& scriptField = actorScriptFields.at(name);

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
						WRITE_SCRIPT_FIELD(Actor, UUID)
						WRITE_SCRIPT_FIELD(AssetHandle, UUID)
					}

					out << YAML::EndMap; // ScriptFields
				}

				out << YAML::EndSeq;
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Actor
	}

	void SceneSerializer::DeserializeActors(const YAML::Node& actorsNode, SharedReference<Scene>& scene)
	{
		for (const YAML::Node& actor : actorsNode)
		{
			uint64_t uuid;

			if (actor["Actor"])
			{
				uuid = actor["Actor"].as<uint64_t>();
			}

			if (actor["Entity"])
			{
				uuid = actor["Entity"].as<uint64_t>();
			}

			bool isActive = true;
			if (actor["Active"])
				VX_DESERIALIZE_PROPERTY(Active, bool, isActive, actor);

			std::string name;
			std::string marker;

			const YAML::Node tagComponent = actor["TagComponent"];

			if (tagComponent)
			{
				if (tagComponent["Tag"])
					name = tagComponent["Tag"].as<std::string>();
				if (tagComponent["Marker"])
					marker = tagComponent["Marker"].as<std::string>();
			}

			Actor deserializedActor = scene->CreateActorWithUUID(uuid, name, marker);
			deserializedActor.SetActive(isActive);

			uint64_t parentHandle = actor["Parent"] ? actor["Parent"].as<uint64_t>() : 0;
			deserializedActor.SetParentUUID(static_cast<UUID>(parentHandle));

			const YAML::Node children = actor["Children"];

			if (children)
			{
				for (const YAML::Node& child : children)
				{
					uint64_t childHandle = child["Handle"].as<uint64_t>();
					deserializedActor.AddChild(static_cast<UUID>(childHandle));
				}
			}

			const YAML::Node transformComponentData = actor["TransformComponent"];
			if (transformComponentData)
			{
				// All Entities have a transform
				TransformComponent& transformComponent = deserializedActor.GetComponent<TransformComponent>();
				transformComponent.Translation = transformComponentData["Translation"].as<Math::vec3>();
				transformComponent.SetRotationEuler(transformComponentData["Rotation"].as<Math::vec3>());
				transformComponent.Scale = transformComponentData["Scale"].as<Math::vec3>();
			}

			const YAML::Node cameraComponentData = actor["CameraComponent"];
			if (cameraComponentData)
			{
				CameraComponent& cameraComponent = deserializedActor.AddComponent<CameraComponent>();

				const YAML::Node cameraProps = cameraComponentData["Camera"];
				cameraComponent.Camera.SetProjectionType((Camera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				cameraComponent.Camera.SetPerspectiveFOV(cameraProps["PerspectiveFOV"].as<float>());
				cameraComponent.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				cameraComponent.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				cameraComponent.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				cameraComponent.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				cameraComponent.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				if (cameraComponentData["ClearColor"])
					cameraComponent.ClearColor = cameraComponentData["ClearColor"].as<Math::vec3>();
				cameraComponent.Primary = cameraComponentData["Primary"].as<bool>();
				cameraComponent.FixedAspectRatio = cameraComponentData["FixedAspectRatio"].as<bool>();
				
				if (cameraComponentData["PostProcessingEnabled"])
					cameraComponent.PostProcessing.Enabled = cameraComponentData["PostProcessingEnabled"].as<bool>();

				if (cameraComponent.PostProcessing.Enabled)
				{
					const YAML::Node postProcessData = cameraComponentData["PostProcessing"];
					
					{
						const YAML::Node bloomData = postProcessData["Bloom"];
						cameraComponent.PostProcessing.Bloom.Threshold = bloomData["Threshold"].as<float>();
						cameraComponent.PostProcessing.Bloom.Knee = bloomData["Knee"].as<float>();
						cameraComponent.PostProcessing.Bloom.Intensity = bloomData["Intensity"].as<float>();
						cameraComponent.PostProcessing.Bloom.Enabled = bloomData["Enabled"].as<float>();
					}
				}
			}

			const YAML::Node skyboxComponentData = actor["SkyboxComponent"];
			if (skyboxComponentData)
			{
				SkyboxComponent& skyboxComponent = deserializedActor.AddComponent<SkyboxComponent>();

				if (skyboxComponentData["Skybox"])
				{
					AssetHandle environmentHandle = skyboxComponentData["Skybox"].as<uint64_t>();
					if (AssetManager::IsHandleValid(environmentHandle))
					{
						skyboxComponent.Skybox = environmentHandle;
					}
				}

				if (skyboxComponentData["Rotation"])
					skyboxComponent.Rotation = skyboxComponentData["Rotation"].as<float>();

				if (skyboxComponentData["Intensity"])
					skyboxComponent.Intensity = skyboxComponentData["Intensity"].as<float>();
			}

			const YAML::Node lightSourceComponentData = actor["LightSourceComponent"];
			if (lightSourceComponentData)
			{
				LightSourceComponent& lightSourceComponent = deserializedActor.AddComponent<LightSourceComponent>();
				
				if (lightSourceComponentData["Visible"])
					lightSourceComponent.Visible = lightSourceComponentData["Visible"].as<bool>();

				lightSourceComponent.Type = Utils::LightTypeFromString(lightSourceComponentData["LightType"].as<std::string>());

				if (lightSourceComponentData["Radiance"])
					lightSourceComponent.Radiance = lightSourceComponentData["Radiance"].as<Math::vec3>();

				switch (lightSourceComponent.Type)
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
						if (lightSourceComponentData["CutOff"])
							lightSourceComponent.Cutoff = lightSourceComponentData["CutOff"].as<float>();
						if (lightSourceComponentData["OuterCutOff"])
							lightSourceComponent.OuterCutoff = lightSourceComponentData["OuterCutOff"].as<float>();

						break;
					}
				}

				if (lightSourceComponentData["Intensity"])
					lightSourceComponent.Intensity = lightSourceComponentData["Intensity"].as<float>();

				if (lightSourceComponentData["ShadowBias"])
					lightSourceComponent.ShadowBias = lightSourceComponentData["ShadowBias"].as<float>();

				if (lightSourceComponentData["CastShadows"])
					lightSourceComponent.CastShadows = lightSourceComponentData["CastShadows"].as<bool>();

				if (lightSourceComponentData["SoftShadows"])
					lightSourceComponent.SoftShadows = lightSourceComponentData["SoftShadows"].as<bool>();
			}

			const YAML::Node meshRendererComponentData = actor["MeshRendererComponent"];
			if (meshRendererComponentData)
			{
				if (meshRendererComponentData["MeshHandle"])
				{
					MeshRendererComponent& meshRendererComponent = deserializedActor.AddComponent<MeshRendererComponent>();
					AssetHandle meshHandle = meshRendererComponentData["MeshHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(meshHandle))
					{
						meshRendererComponent.Mesh = meshHandle;

						// Move this to asset serializer
						/*MeshImportOptions importOptions = MeshImportOptions();
						if (meshComponent["MeshImportOptions"])
						{
							ModelImportOptions modelImportOptions = meshComponent["ModelImportOptions"];
							importOptions.MeshTransformation.Translation = modelImportOptions["Translation"].as<Math::vec3>();
							importOptions.MeshTransformation.SetRotationEuler(modelImportOptions["Rotation"].as<Math::vec3>());
							importOptions.MeshTransformation.Scale = modelImportOptions["Scale"].as<Math::vec3>();
						}*/

						if (meshRendererComponentData["Visible"])
							meshRendererComponent.Visible = meshRendererComponentData["Visible"].as<bool>();
						if (meshRendererComponentData["CastShadows"])
							meshRendererComponent.CastShadows = meshRendererComponentData["CastShadows"].as<bool>();
					}
				}
			}

			const YAML::Node staticMeshRendererComponentData = actor["StaticMeshRendererComponent"];
			if (staticMeshRendererComponentData)
			{
				StaticMeshRendererComponent& staticMeshRendererComponent = deserializedActor.AddComponent<StaticMeshRendererComponent>();
				staticMeshRendererComponent.Type = Utils::MeshTypeFromString(staticMeshRendererComponentData["MeshType"].as<std::string>());

				if (staticMeshRendererComponent.Type != MeshType::Custom)
				{
					DefaultMesh::StaticMeshType defaultMesh = (DefaultMesh::StaticMeshType)staticMeshRendererComponent.Type;
					staticMeshRendererComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(defaultMesh);
				}
				else
				{
					if (staticMeshRendererComponentData["MeshHandle"])
					{
						AssetHandle staticMeshHandle = staticMeshRendererComponentData["MeshHandle"].as<uint64_t>();

						if (AssetManager::IsHandleValid(staticMeshHandle))
						{
							staticMeshRendererComponent.StaticMesh = staticMeshHandle;

							// Move this to asset serializer
							/*MeshImportOptions importOptions = MeshImportOptions();
							if (staticMeshComponentData["MeshImportOptions"])
							{
								ModelImportOptions modelImportOptions = staticMeshComponent["MeshImportOptions"];
								importOptions.MeshTransformation.Translation = modelImportOptions["Translation"].as<Math::vec3>();
								importOptions.MeshTransformation.SetRotationEuler(modelImportOptions["Rotation"].as<Math::vec3>());
								importOptions.MeshTransformation.Scale = modelImportOptions["Scale"].as<Math::vec3>();
							}*/

							if (staticMeshRendererComponentData["Visible"])
								staticMeshRendererComponent.Visible = staticMeshRendererComponentData["Visible"].as<bool>();
							if (staticMeshRendererComponentData["CastShadows"])
								staticMeshRendererComponent.CastShadows = staticMeshRendererComponentData["CastShadows"].as<bool>();
						}
					}
				}

				// Load materials
				if (AssetManager::IsHandleValid(staticMeshRendererComponent.StaticMesh))
				{
					// Do this in Asset Serializer
					const YAML::Node submeshesData = staticMeshRendererComponentData["Submeshes"];
					if (submeshesData)
					{
						SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshRendererComponent.StaticMesh);

						if (staticMesh)
						{
							SharedReference<MaterialTable> materialTable = staticMeshRendererComponent.Materials;

							uint32_t submeshIndex = 0;
							for (const YAML::Node& submeshData : submeshesData)
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

								// Load material textures so it is ready to go for rendering
								SharedReference<Material> material = AssetManager::GetAsset<Material>(materialHandle);

								// Do we really need to do this? won't the renderer grab them anyway?
								/*AssetManager::GetAsset<Texture2D>(material->GetAlbedoMap());
								AssetManager::GetAsset<Texture2D>(material->GetNormalMap());
								AssetManager::GetAsset<Texture2D>(material->GetMetallicMap());
								AssetManager::GetAsset<Texture2D>(material->GetRoughnessMap());
								AssetManager::GetAsset<Texture2D>(material->GetEmissionMap());
								AssetManager::GetAsset<Texture2D>(material->GetAmbientOcclusionMap());*/

								materialTable->SetMaterial(submeshIndex++, materialHandle);
							}
						}
					}
				}
			}

			const YAML::Node spriteRendererComponentData = actor["SpriteRendererComponent"];
			if (spriteRendererComponentData)
			{
				SpriteRendererComponent& spriteRendererComponent = deserializedActor.AddComponent<SpriteRendererComponent>();
				spriteRendererComponent.SpriteColor = spriteRendererComponentData["Color"].as<Math::vec4>();

				if (spriteRendererComponentData["Visible"])
					spriteRendererComponent.Visible = spriteRendererComponentData["Visible"].as<bool>();

				if (spriteRendererComponentData["TextureHandle"])
				{
					AssetHandle assetHandle = spriteRendererComponentData["TextureHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(assetHandle))
					{
						spriteRendererComponent.Texture = assetHandle;
					}

					if (spriteRendererComponentData["TextureScale"])
						spriteRendererComponent.TextureUV = spriteRendererComponentData["TextureScale"].as<Math::vec2>();
				}
			}

			const YAML::Node circleRendererComponentData = actor["CircleRendererComponent"];
			if (circleRendererComponentData)
			{
				CircleRendererComponent& circleRendererComponent = deserializedActor.AddComponent<CircleRendererComponent>();

				if (circleRendererComponentData["Visible"])
					circleRendererComponent.Visible = circleRendererComponentData["Visible"].as<bool>();

				circleRendererComponent.Color = circleRendererComponentData["Color"].as<Math::vec4>();
				circleRendererComponent.Thickness = circleRendererComponentData["Thickness"].as<float>();
				circleRendererComponent.Fade = circleRendererComponentData["Fade"].as<float>();
			}

			const YAML::Node particleEmitterComponentData = actor["ParticleEmitterComponent"];
			if (particleEmitterComponentData)
			{
				ParticleEmitterComponent& particleEmitterComponent = deserializedActor.AddComponent<ParticleEmitterComponent>();

				if (particleEmitterComponentData["EmitterHandle"])
				{
					AssetHandle emitterHandle = particleEmitterComponentData["EmitterHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(emitterHandle))
					{
						particleEmitterComponent.EmitterHandle = emitterHandle;
					}
				}

				if (particleEmitterComponentData["IsActive"])
				{
					particleEmitterComponent.IsActive = particleEmitterComponentData["IsActive"].as<bool>();
				}
				else
				{
					particleEmitterComponent.IsActive = false;
				}
			}

			const YAML::Node textMeshComponentData = actor["TextMeshComponent"];
			if (textMeshComponentData)
			{
				TextMeshComponent& textMeshComponent = deserializedActor.AddComponent<TextMeshComponent>();

				if (textMeshComponentData["Visible"])
					textMeshComponent.Visible = textMeshComponentData["Visible"].as<bool>();

				if (textMeshComponentData["FontHandle"])
				{
					AssetHandle fontHandle = textMeshComponentData["FontHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(fontHandle))
					{
						textMeshComponent.FontAsset = fontHandle;
					}
				}

				textMeshComponent.Color = textMeshComponentData["Color"].as<Math::vec4>();
				if (textMeshComponentData["BackgroundColor"])
					textMeshComponent.BackgroundColor = textMeshComponentData["BackgroundColor"].as<Math::vec4>();
				textMeshComponent.Kerning = textMeshComponentData["Kerning"].as<float>();
				textMeshComponent.LineSpacing = textMeshComponentData["LineSpacing"].as<float>();
				textMeshComponent.MaxWidth = textMeshComponentData["MaxWidth"].as<float>();
				textMeshComponent.TextHash = textMeshComponentData["TextHash"].as<size_t>();
				textMeshComponent.TextString = textMeshComponentData["TextString"].as<std::string>();
				
				if (textMeshComponentData["DropShadowEnabled"])
					textMeshComponent.DropShadow.Enabled = textMeshComponentData["DropShadowEnabled"].as<bool>();

				if (textMeshComponent.DropShadow.Enabled)
				{
					const YAML::Node dropShadowData = textMeshComponentData["DropShadow"];
					textMeshComponent.DropShadow.Color = dropShadowData["Color"].as<Math::vec4>();
					textMeshComponent.DropShadow.ShadowDistance = dropShadowData["ShadowDistance"].as<Math::vec2>();
					textMeshComponent.DropShadow.ShadowScale = dropShadowData["ShadowScale"].as<float>();
				}
			}

			const YAML::Node animationComponent = actor["AnimationComponent"];
			if (animationComponent)
			{
				if (!deserializedActor.HasComponent<MeshRendererComponent>())
				{
					VX_CONSOLE_LOG_WARN("Trying to add Animation Component without Mesh Renderer Component!");
					return;
				}

				// TODO fix animations to take in mesh asset handle
				/*AnimationComponent& animationComponent = deserializedActor.AddComponent<AnimationComponent>();
				SharedRef<Mesh> mesh = deserializedActor.GetComponent<MeshRendererComponent>().Mesh;
				std::string filepath = mesh->GetPath();
				animationComponent.Animation = Animation::Create(filepath, mesh);*/
			}

			const YAML::Node animatorComponentData = actor["AnimatorComponent"];
			if (animatorComponentData)
			{
				if (!deserializedActor.HasComponent<AnimationComponent>())
				{
					VX_CONSOLE_LOG_WARN("Trying to add Animator Component without Animation Component!");
					return;
				}

				AnimatorComponent& animatorComponent = deserializedActor.AddComponent<AnimatorComponent>();
				animatorComponent.Animator = Animator::Create(deserializedActor.GetComponent<AnimationComponent>().Animation);
			}

			const YAML::Node audioSourceComponentData = actor["AudioSourceComponent"];
			if (audioSourceComponentData)
			{
				AudioSourceComponent& audioSourceComponent = deserializedActor.AddComponent<AudioSourceComponent>();

				AssetHandle audioHandle = audioSourceComponentData["AudioHandle"].as<uint64_t>();
				if (AssetManager::IsHandleValid(audioHandle))
				{
					audioSourceComponent.AudioHandle = audioHandle;
				}

				if (audioSourceComponentData["PlayOnStart"])
				{
					audioSourceComponent.PlayOnStart = audioSourceComponentData["PlayOnStart"].as<bool>();
				}
				else
				{
					audioSourceComponent.PlayOnStart = false;
				}

				if (audioSourceComponentData["PlayOneShot"])
				{
					audioSourceComponent.PlayOneShot = audioSourceComponentData["PlayOneShot"].as<bool>();
				}
				else
				{
					audioSourceComponent.PlayOneShot = false;
				}
			}

			const YAML::Node audioListenerComponentData = actor["AudioListenerComponent"];
			if (audioListenerComponentData)
			{
				// TODO
			}

			const YAML::Node rigidbodyComponentData = actor["RigidbodyComponent"];
			if (rigidbodyComponentData)
			{
				RigidBodyComponent& rigidbodyComponent = deserializedActor.AddComponent<RigidBodyComponent>();

				rigidbodyComponent.Type = Utils::RigidBodyTypeFromString(rigidbodyComponentData["BodyType"].as<std::string>());
				if (rigidbodyComponentData["Mass"])
					rigidbodyComponent.Mass = rigidbodyComponentData["Mass"].as<float>();
				if (rigidbodyComponentData["AngularDrag"])
					rigidbodyComponent.AngularDrag = rigidbodyComponentData["AngularDrag"].as<float>();
				if (rigidbodyComponentData["MaxAngularVelocity"])
					rigidbodyComponent.MaxAngularVelocity = rigidbodyComponentData["MaxAngularVelocity"].as<float>();
				if (rigidbodyComponentData["AngularVelocity"])
					rigidbodyComponent.AngularVelocity = rigidbodyComponentData["AngularVelocity"].as<Math::vec3>();
				if (rigidbodyComponentData["DisableGravity"])
					rigidbodyComponent.DisableGravity = rigidbodyComponentData["DisableGravity"].as<bool>();
				if (rigidbodyComponentData["IsKinematic"])
					rigidbodyComponent.IsKinematic = rigidbodyComponentData["IsKinematic"].as<bool>();
				if (rigidbodyComponentData["LinearDrag"])
					rigidbodyComponent.LinearDrag = rigidbodyComponentData["LinearDrag"].as<float>();
				if (rigidbodyComponentData["MaxLinearVelocity"])
					rigidbodyComponent.MaxLinearVelocity = rigidbodyComponentData["MaxLinearVelocity"].as<float>();
				if (rigidbodyComponentData["LinearVelocity"])
					rigidbodyComponent.LinearVelocity = rigidbodyComponentData["LinearVelocity"].as<Math::vec3>();
				if (rigidbodyComponentData["CollisionDetectionType"])
					rigidbodyComponent.CollisionDetection = Utils::CollisionDetectionTypeFromString(rigidbodyComponentData["CollisionDetectionType"].as<std::string>());
				if (rigidbodyComponentData["ActorLockFlags"])
					rigidbodyComponent.LockFlags = rigidbodyComponentData["ActorLockFlags"].as<uint32_t>(0);
			}

			const YAML::Node characterControllerComponentData = actor["CharacterControllerComponent"];
			if (characterControllerComponentData)
			{
				CharacterControllerComponent& characterControllerComponent = deserializedActor.AddComponent<CharacterControllerComponent>();

				if (characterControllerComponentData["NonWalkableMode"])
					characterControllerComponent.NonWalkMode = Utils::NonWalkableModeFromString(characterControllerComponentData["NonWalkableMode"].as<std::string>());
				if (characterControllerComponentData["CapsuleClimbMode"])
					characterControllerComponent.ClimbMode = Utils::CapsuleClimbModeFromString(characterControllerComponentData["CapsuleClimbMode"].as<std::string>());
				characterControllerComponent.DisableGravity = characterControllerComponentData["DisableGravity"].as<bool>();
				characterControllerComponent.LayerID = characterControllerComponentData["LayerID"].as<uint32_t>();
				characterControllerComponent.SlopeLimitDegrees = characterControllerComponentData["SlopeLimitDegrees"].as<float>();
				characterControllerComponent.StepOffset = characterControllerComponentData["StepOffset"].as<float>();
				if (characterControllerComponentData["ContactOffset"])
					characterControllerComponent.ContactOffset = characterControllerComponentData["ContactOffset"].as<float>();
			}

			const YAML::Node fixedJointComponentData = actor["FixedJointComponent"];
			if (fixedJointComponentData)
			{
				FixedJointComponent& fixedJointComponent = deserializedActor.AddComponent<FixedJointComponent>();

				VX_DESERIALIZE_PROPERTY(ConnectedActor, uint64_t, fixedJointComponent.ConnectedActor, fixedJointComponentData);
				VX_DESERIALIZE_PROPERTY(BreakForce, float, fixedJointComponent.BreakForce, fixedJointComponentData);
				VX_DESERIALIZE_PROPERTY(BreakTorque, float, fixedJointComponent.BreakTorque, fixedJointComponentData);
				VX_DESERIALIZE_PROPERTY(EnableCollision, bool, fixedJointComponent.EnableCollision, fixedJointComponentData);
				VX_DESERIALIZE_PROPERTY(EnablePreProcessing, bool, fixedJointComponent.EnablePreProcessing, fixedJointComponentData);
				VX_DESERIALIZE_PROPERTY(IsBreakable, bool, fixedJointComponent.IsBreakable, fixedJointComponentData);
			}

			auto DeserializePhysicsMaterialFn = [](auto physicsMaterialData) {
				AssetHandle materialHandle = AssetManager::CreateMemoryOnlyAsset<PhysicsMaterial>();
				SharedReference<PhysicsMaterial> physicsMaterial = AssetManager::GetAsset<PhysicsMaterial>(materialHandle);

				physicsMaterial->StaticFriction = physicsMaterialData["StaticFriction"].as<float>();
				physicsMaterial->DynamicFriction = physicsMaterialData["DynamicFriction"].as<float>();
				physicsMaterial->Bounciness = physicsMaterialData["Bounciness"].as<float>();

				if (physicsMaterialData["FrictionCombineMode"])
					physicsMaterial->FrictionCombineMode = Utils::CombineModeFromString(physicsMaterialData["FrictionCombineMode"].as<std::string>());
				if (physicsMaterialData["BouncinessCombineMode"])
					physicsMaterial->BouncinessCombineMode = Utils::CombineModeFromString(physicsMaterialData["BouncinessCombineMode"].as<std::string>());
			};

			const YAML::Node boxColliderComponentData = actor["BoxColliderComponent"];
			if (boxColliderComponentData)
			{
				BoxColliderComponent& boxColliderComponent = deserializedActor.AddComponent<BoxColliderComponent>();

				boxColliderComponent.HalfSize = boxColliderComponentData["HalfSize"].as<glm::vec3>();
				boxColliderComponent.Offset = boxColliderComponentData["Offset"].as<glm::vec3>();
				if (boxColliderComponentData["IsTrigger"])
					boxColliderComponent.IsTrigger = boxColliderComponentData["IsTrigger"].as<bool>();
				if (boxColliderComponentData["Visible"])
					boxColliderComponent.Visible = boxColliderComponentData["Visible"].as<bool>();

				const YAML::Node physicsMaterialData = boxColliderComponentData["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					DeserializePhysicsMaterialFn(physicsMaterialData);
				}
			}

			const YAML::Node sphereColliderComponentData = actor["SphereColliderComponent"];
			if (sphereColliderComponentData)
			{
				SphereColliderComponent& sphereColliderComponent = deserializedActor.AddComponent<SphereColliderComponent>();

				sphereColliderComponent.Radius = sphereColliderComponentData["Radius"].as<float>();
				sphereColliderComponent.Offset = sphereColliderComponentData["Offset"].as<Math::vec3>();
				sphereColliderComponent.IsTrigger = sphereColliderComponentData["IsTrigger"].as<bool>();
				if (sphereColliderComponentData["Visible"])
					sphereColliderComponent.Visible = sphereColliderComponentData["Visible"].as<bool>();

				const YAML::Node physicsMaterialData = sphereColliderComponentData["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					DeserializePhysicsMaterialFn(physicsMaterialData);
				}
			}

			const YAML::Node capsuleColliderComponentData = actor["CapsuleColliderComponent"];
			if (capsuleColliderComponentData)
			{
				CapsuleColliderComponent& capsuleColliderComponent = deserializedActor.AddComponent<CapsuleColliderComponent>();

				capsuleColliderComponent.Radius = capsuleColliderComponentData["Radius"].as<float>();
				capsuleColliderComponent.Height = capsuleColliderComponentData["Height"].as<float>();
				capsuleColliderComponent.Offset = capsuleColliderComponentData["Offset"].as<Math::vec3>();
				capsuleColliderComponent.IsTrigger = capsuleColliderComponentData["IsTrigger"].as<bool>();
				if (capsuleColliderComponentData["Visible"])
					capsuleColliderComponent.Visible = capsuleColliderComponentData["Visible"].as<bool>();

				const YAML::Node physicsMaterialData = capsuleColliderComponentData["PhysicsMaterial"];

				if (physicsMaterialData)
				{
					DeserializePhysicsMaterialFn(physicsMaterialData);
				}
			}

			const YAML::Node meshColliderComponentData = actor["MeshColliderComponent"];
			if (meshColliderComponentData)
			{
				MeshColliderComponent& meshColliderComponent = deserializedActor.AddComponent<MeshColliderComponent>();

				// TODO
			}

			const YAML::Node rigidBody2DComponentData = actor["Rigidbody2DComponent"];
			if (rigidBody2DComponentData)
			{
				RigidBody2DComponent& rigidBodyComponent = deserializedActor.AddComponent<RigidBody2DComponent>();

				rigidBodyComponent.Type = Utils::RigidBody2DBodyTypeFromString(rigidBody2DComponentData["BodyType"].as<std::string>());
				if (rigidBody2DComponentData["Velocity"])
					rigidBodyComponent.Velocity = rigidBody2DComponentData["Velocity"].as<Math::vec2>();
				if (rigidBody2DComponentData["Drag"])
					rigidBodyComponent.Drag = rigidBody2DComponentData["Drag"].as<float>();
				if (rigidBody2DComponentData["AngularVelocity"])
					rigidBodyComponent.AngularVelocity = rigidBody2DComponentData["AngularVelocity"].as<float>();
				if (rigidBody2DComponentData["AngularDrag"])
					rigidBodyComponent.AngularDrag = rigidBody2DComponentData["AngularDrag"].as<float>();
				if (rigidBody2DComponentData["GravityScale"])
					rigidBodyComponent.GravityScale = rigidBody2DComponentData["GravityScale"].as<float>();
				if (rigidBody2DComponentData["FreezeRotation"])
					rigidBodyComponent.FixedRotation = rigidBody2DComponentData["FreezeRotation"].as<bool>();
			}

			const YAML::Node boxCollider2DComponentData = actor["BoxCollider2DComponent"];
			if (boxCollider2DComponentData)
			{
				BoxCollider2DComponent& boxColliderComponent = deserializedActor.AddComponent<BoxCollider2DComponent>();

				boxColliderComponent.Offset = boxCollider2DComponentData["Offset"].as<glm::vec2>();
				boxColliderComponent.Size = boxCollider2DComponentData["Size"].as<glm::vec2>();
				boxColliderComponent.Density = boxCollider2DComponentData["Density"].as<float>();
				boxColliderComponent.Friction = boxCollider2DComponentData["Friction"].as<float>();
				boxColliderComponent.Restitution = boxCollider2DComponentData["Restitution"].as<float>();
				boxColliderComponent.RestitutionThreshold = boxCollider2DComponentData["RestitutionThreshold"].as<float>();
				if (boxCollider2DComponentData["IsTrigger"])
					boxColliderComponent.IsTrigger = boxCollider2DComponentData["IsTrigger"].as<bool>();
			}

			const YAML::Node circleCollider2DComponentData = actor["CircleCollider2DComponent"];
			if (circleCollider2DComponentData)
			{
				CircleCollider2DComponent& circleColliderComponent = deserializedActor.AddComponent<CircleCollider2DComponent>();

				circleColliderComponent.Offset = circleCollider2DComponentData["Offset"].as<glm::vec2>();
				circleColliderComponent.Radius = circleCollider2DComponentData["Radius"].as<float>();
				circleColliderComponent.Density = circleCollider2DComponentData["Density"].as<float>();
				circleColliderComponent.Friction = circleCollider2DComponentData["Friction"].as<float>();
				circleColliderComponent.Restitution = circleCollider2DComponentData["Restitution"].as<float>();
				circleColliderComponent.RestitutionThreshold = circleCollider2DComponentData["RestitutionThreshold"].as<float>();
			}

			const YAML::Node scriptComponentData = actor["ScriptComponent"];
			if (scriptComponentData)
			{
				ScriptComponent& scriptComponent = deserializedActor.AddComponent<ScriptComponent>();
				scriptComponent.ClassName = scriptComponentData["ClassName"].as<std::string>();
				if (scriptComponentData["Enabled"])
					scriptComponent.Enabled = scriptComponentData["Enabled"].as<bool>();
				else
					scriptComponent.Enabled = true;

				if (ScriptEngine::ScriptClassExists(scriptComponent.ClassName))
				{
					const YAML::Node scriptFieldData = scriptComponentData["ScriptFields"];

					if (scriptFieldData)
					{
						SharedReference<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(scriptComponent.ClassName);

						const std::map<std::string, ScriptField>& classFields = scriptClass->GetFields();
						ScriptFieldMap& scriptFields = ScriptEngine::GetMutableScriptFieldMap(deserializedActor);

						for (const YAML::Node& scriptField : scriptFieldData)
						{
							const std::string fieldName = scriptField["Name"].as<std::string>();

							ScriptFieldInstance& fieldInstance = scriptFields[fieldName];

							if (classFields.find(fieldName) == classFields.end())
							{
								VX_CONSOLE_LOG_WARN("Script Field '{}' was not found in Field Map!", fieldName);
								continue;
							}

							fieldInstance.Field = classFields.at(fieldName);

							const std::string typeString = scriptField["Type"].as<std::string>();
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
								READ_SCRIPT_FIELD(Actor, UUID)
								READ_SCRIPT_FIELD(AssetHandle, UUID)
							}
						}
					}	
				}
			}
		}
	}

}
