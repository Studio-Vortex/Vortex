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
			return false;

		const std::string sceneName = data["Scene"].as<std::string>();
		VX_CONSOLE_LOG_TRACE("Deserializing Scene '{}'", sceneName);

		auto actors = data["Actors"];
		if (actors)
		{
			DeserializeActors(actors, m_Scene);
		}
		else
		{
			// for backwards compatability
			actors = data["Entities"];
			DeserializeActors(actors, m_Scene);
		}

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
			const auto& hierarchyComponent = actor.GetComponent<HierarchyComponent>();
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

		if (actor.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent" << YAML::Value << YAML::BeginMap; // TagComponent

			const auto& tagComponent = actor.GetComponent<TagComponent>();

			VX_SERIALIZE_PROPERTY(Tag, tagComponent.Tag, out);
			VX_SERIALIZE_PROPERTY(Marker, tagComponent.Marker, out);

			out << YAML::EndMap; // TagComponent
		}

		if (actor.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent" << YAML::Value << YAML::BeginMap; // TransformComponent

			const auto& transform = actor.GetComponent<TransformComponent>();

			VX_SERIALIZE_PROPERTY(Translation, transform.Translation, out);
			VX_SERIALIZE_PROPERTY(Rotation, transform.GetRotationEuler(), out);
			VX_SERIALIZE_PROPERTY(Scale, transform.Scale, out);

			out << YAML::EndMap; // TransformComponent
		}

		if (actor.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent" << YAML::BeginMap; // CameraComponent

			const auto& cameraComponent = actor.GetComponent<CameraComponent>();
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
			VX_SERIALIZE_PROPERTY(PostProcessingEnabled, cameraComponent.PostProcessing.Enabled, out);

			if (cameraComponent.PostProcessing.Enabled)
			{
				out << YAML::Key << "PostProcessing" << YAML::Value;

				out << YAML::BeginMap; // PostProcessing
				{
					out << YAML::Key << "Bloom" << YAML::Value;
					out << YAML::BeginMap; // Bloom
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
			const auto& skyboxComponent = actor.GetComponent<SkyboxComponent>();
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

			const auto& lightComponent = actor.GetComponent<LightSourceComponent>();

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

		if (actor.HasComponent<MeshRendererComponent>())
		{
			const auto& meshRendererComponent = actor.GetComponent<MeshRendererComponent>();
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

		if (actor.HasComponent<StaticMeshRendererComponent>())
		{
			const auto& staticMeshRenderer = actor.GetComponent<StaticMeshRendererComponent>();
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

						for (const auto& [submeshIndex, submesh] : submeshes)
						{
							AssetHandle materialHandle = staticMeshRenderer.Materials->GetMaterial(submeshIndex);
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
			const auto& spriteComponent = actor.GetComponent<SpriteRendererComponent>();

			out << YAML::Key << "SpriteRendererComponent" << YAML::Value << YAML::BeginMap; // SpriteRendererComponent

			AssetHandle spriteHandle = spriteComponent.Texture;
			VX_SERIALIZE_PROPERTY(TextureHandle, spriteHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, spriteComponent.Visible, out);

			VX_SERIALIZE_PROPERTY(Color, spriteComponent.SpriteColor, out);
			VX_SERIALIZE_PROPERTY(TextureUV, spriteComponent.TextureUV, out);

			out << YAML::EndMap; // SpriteRendererComponent
		}

		if (actor.HasComponent<CircleRendererComponent>())
		{
			out << YAML::Key << "CircleRendererComponent" << YAML::Value << YAML::BeginMap; // CircleRendererComponent

			const auto& circleComponent = actor.GetComponent<CircleRendererComponent>();

			VX_SERIALIZE_PROPERTY(Visible, circleComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, circleComponent.Color, out);
			VX_SERIALIZE_PROPERTY(Thickness, circleComponent.Thickness, out);
			VX_SERIALIZE_PROPERTY(Fade, circleComponent.Fade, out);

			out << YAML::EndMap; // CircleRendererComponent
		}

		if (actor.HasComponent<ParticleEmitterComponent>())
		{
			const auto& pmc = actor.GetComponent<ParticleEmitterComponent>();
			AssetHandle emitterHandle = pmc.EmitterHandle;

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

			VX_SERIALIZE_PROPERTY(IsActive, pmc.IsActive, out);

			out << YAML::EndMap; // ParticleEmitterComponent
		}

		if (actor.HasComponent<TextMeshComponent>())
		{
			const auto& textMeshComponent = actor.GetComponent<TextMeshComponent>();

			out << YAML::Key << "TextMeshComponent" << YAML::Value << YAML::BeginMap; // TextMeshComponent

			AssetHandle fontHandle = textMeshComponent.FontAsset;

			VX_SERIALIZE_PROPERTY(FontHandle, fontHandle, out);
			VX_SERIALIZE_PROPERTY(Visible, textMeshComponent.Visible, out);
			VX_SERIALIZE_PROPERTY(Color, textMeshComponent.Color, out);
			VX_SERIALIZE_PROPERTY(BackgroundColor, textMeshComponent.BackgroundColor, out);
			VX_SERIALIZE_PROPERTY(Kerning, textMeshComponent.Kerning, out);
			VX_SERIALIZE_PROPERTY(LineSpacing, textMeshComponent.LineSpacing, out);
			VX_SERIALIZE_PROPERTY(MaxWidth, textMeshComponent.MaxWidth, out);
			VX_SERIALIZE_PROPERTY(TextHash, textMeshComponent.TextHash, out);
			VX_SERIALIZE_PROPERTY(TextString, textMeshComponent.TextString, out);

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
			const auto& asc = actor.GetComponent<AudioSourceComponent>();

			out << YAML::Key << "AudioSourceComponent" << YAML::Value << YAML::BeginMap; //AudioSourceComponent
			if (AssetManager::IsHandleValid(asc.AudioHandle))
			{
				SharedReference<AudioSource> audioSource = AssetManager::GetAsset<AudioSource>(asc.AudioHandle);
				
				if (audioSource)
				{
					AssetImporter::Serialize(audioSource);

					VX_SERIALIZE_PROPERTY(AudioHandle, asc.AudioHandle, out);
				}
			}

			VX_SERIALIZE_PROPERTY(PlayOnStart, asc.PlayOnStart, out);
			VX_SERIALIZE_PROPERTY(PlayOneShot, asc.PlayOneShot, out);

			out << YAML::EndMap; // AudioSourceComponent
		}

		if (actor.HasComponent<AudioListenerComponent>())
		{
			// TODO
		}

		if (actor.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidbodyComponent" << YAML::BeginMap; // RigidbodyComponent

			const auto& rigidbodyComponent = actor.GetComponent<RigidBodyComponent>();
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

		if (actor.HasComponent<CharacterControllerComponent>())
		{
			out << YAML::Key << "CharacterControllerComponent" << YAML::BeginMap; // CharacterControllerComponent

			const auto& characterController = actor.GetComponent<CharacterControllerComponent>();
			VX_SERIALIZE_PROPERTY(NonWalkableMode, Utils::NonWalkableModeToString(characterController.NonWalkMode), out);
			VX_SERIALIZE_PROPERTY(CapsuleClimbMode, Utils::CapsuleClimbModeToString(characterController.ClimbMode), out);
			VX_SERIALIZE_PROPERTY(DisableGravity, characterController.DisableGravity, out);
			VX_SERIALIZE_PROPERTY(LayerID, characterController.LayerID, out);
			VX_SERIALIZE_PROPERTY(SlopeLimitDegrees, characterController.SlopeLimitDegrees, out);
			VX_SERIALIZE_PROPERTY(StepOffset, characterController.StepOffset, out);
			VX_SERIALIZE_PROPERTY(ContactOffset, characterController.ContactOffset, out);

			out << YAML::EndMap; // CharacterControllerComponent
		}

		if (actor.HasComponent<FixedJointComponent>())
		{
			out << YAML::Key << "FixedJointComponent" << YAML::BeginMap; // FixedJointComponent

			const auto& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
			VX_SERIALIZE_PROPERTY(ConnectedActor, fixedJointComponent.ConnectedActor, out);
			VX_SERIALIZE_PROPERTY(BreakForce, fixedJointComponent.BreakForce, out);
			VX_SERIALIZE_PROPERTY(BreakTorque, fixedJointComponent.BreakTorque, out);
			VX_SERIALIZE_PROPERTY(EnableCollision, fixedJointComponent.EnableCollision, out);
			VX_SERIALIZE_PROPERTY(EnablePreProcessing, fixedJointComponent.EnablePreProcessing, out);
			VX_SERIALIZE_PROPERTY(IsBreakable, fixedJointComponent.IsBreakable, out);

			out << YAML::EndMap; // FixedJointComponent
		}
		
		if (actor.HasComponent<BoxColliderComponent>())
		{
			out << YAML::Key << "BoxColliderComponent" << YAML::BeginMap; // BoxColliderComponent

			const auto& boxColliderComponent = actor.GetComponent<BoxColliderComponent>();
			VX_SERIALIZE_PROPERTY(HalfSize, boxColliderComponent.HalfSize, out);
			VX_SERIALIZE_PROPERTY(Offset, boxColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, boxColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, boxColliderComponent.Visible, out);

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

		if (actor.HasComponent<SphereColliderComponent>())
		{
			out << YAML::Key << "SphereColliderComponent" << YAML::BeginMap; // SphereColliderComponent

			const auto& sphereColliderComponent = actor.GetComponent<SphereColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, sphereColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Offset, sphereColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, sphereColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, sphereColliderComponent.Visible, out);

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

		if (actor.HasComponent<CapsuleColliderComponent>())
		{
			out << YAML::Key << "CapsuleColliderComponent" << YAML::BeginMap; // CapsuleColliderComponent

			const auto& capsuleColliderComponent = actor.GetComponent<CapsuleColliderComponent>();
			VX_SERIALIZE_PROPERTY(Radius, capsuleColliderComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Height, capsuleColliderComponent.Height, out);
			VX_SERIALIZE_PROPERTY(Offset, capsuleColliderComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, capsuleColliderComponent.IsTrigger, out);
			VX_SERIALIZE_PROPERTY(Visible, capsuleColliderComponent.Visible, out);

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

		if (actor.HasComponent<MeshColliderComponent>())
		{
			// TODO
		}

		if (actor.HasComponent<RigidBody2DComponent>())
		{
			out << YAML::Key << "Rigidbody2DComponent" << YAML::BeginMap; // Rigidbody2DComponent

			const auto& rb2dComponent = actor.GetComponent<RigidBody2DComponent>();
			VX_SERIALIZE_PROPERTY(BodyType, Utils::RigidBody2DBodyTypeToString(rb2dComponent.Type), out);
			VX_SERIALIZE_PROPERTY(Velocity, rb2dComponent.Velocity, out);
			VX_SERIALIZE_PROPERTY(Drag, rb2dComponent.Drag, out);
			VX_SERIALIZE_PROPERTY(AngularVelocity, rb2dComponent.AngularVelocity, out);
			VX_SERIALIZE_PROPERTY(AngularDrag, rb2dComponent.AngularDrag, out);
			VX_SERIALIZE_PROPERTY(GravityScale, rb2dComponent.GravityScale, out);
			VX_SERIALIZE_PROPERTY(FreezeRotation, rb2dComponent.FixedRotation, out);

			out << YAML::EndMap; // Rigidbody2DComponent
		}

		if (actor.HasComponent<BoxCollider2DComponent>())
		{
			out << YAML::Key << "BoxCollider2DComponent" << YAML::BeginMap; // BoxCollider2DComponent

			const auto& bc2dComponent = actor.GetComponent<BoxCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, bc2dComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Size, bc2dComponent.Size, out);
			VX_SERIALIZE_PROPERTY(Density, bc2dComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, bc2dComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, bc2dComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, bc2dComponent.RestitutionThreshold, out);
			VX_SERIALIZE_PROPERTY(IsTrigger, bc2dComponent.IsTrigger, out);

			out << YAML::EndMap; // BoxCollider2DComponent
		}

		if (actor.HasComponent<CircleCollider2DComponent>())
		{
			out << YAML::Key << "CircleCollider2DComponent" << YAML::BeginMap; // CircleCollider2DComponent

			const auto& cc2dComponent = actor.GetComponent<CircleCollider2DComponent>();
			VX_SERIALIZE_PROPERTY(Offset, cc2dComponent.Offset, out);
			VX_SERIALIZE_PROPERTY(Radius, cc2dComponent.Radius, out);
			VX_SERIALIZE_PROPERTY(Density, cc2dComponent.Density, out);
			VX_SERIALIZE_PROPERTY(Friction, cc2dComponent.Friction, out);
			VX_SERIALIZE_PROPERTY(Restitution, cc2dComponent.Restitution, out);
			VX_SERIALIZE_PROPERTY(RestitutionThreshold, cc2dComponent.RestitutionThreshold, out);

			out << YAML::EndMap; // CircleCollider2DComponent
		}

		if (actor.HasComponent<NavMeshAgentComponent>())
		{
			out << YAML::Key << "NavMeshAgentComponent" << YAML::BeginMap; // NavMeshAgentComponent

			const auto& navMeshComponent = actor.GetComponent<NavMeshAgentComponent>();

			out << YAML::EndMap; // NavMeshAgentComponent
		}

		if (actor.HasComponent<ScriptComponent>() && !actor.GetComponent<ScriptComponent>().ClassName.empty())
		{
			const auto& scriptComponent = actor.GetComponent<ScriptComponent>();

			// Script Class Fields
			if (ScriptEngine::ActorClassExists(scriptComponent.ClassName))
			{
				out << YAML::Key << "ScriptComponent" << YAML::BeginMap; // ScriptComponent
				VX_SERIALIZE_PROPERTY(ClassName, scriptComponent.ClassName, out);

				SharedReference<ScriptClass> actorClass = ScriptEngine::GetActorClass(scriptComponent.ClassName);
				const auto& fields = actorClass->GetFields();

				if (fields.size() > 0)
				{
					const ScriptFieldMap& actorScriptFields = ScriptEngine::GetScriptFieldMap(actor);

					out << YAML::Key << "ScriptFields" << YAML::Value;
					out << YAML::BeginSeq;

					for (const auto& [name, field] : fields)
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
			}

			out << YAML::EndMap; // ScriptComponent
		}

		out << YAML::EndMap; // Actor
	}

	void SceneSerializer::DeserializeActors(YAML::Node& actorsNode, SharedReference<Scene>& scene)
	{
		for (auto actor : actorsNode)
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

			auto tagComponent = actor["TagComponent"];

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

			const auto children = actor["Children"];

			if (children)
			{
				for (auto& child : children)
				{
					uint64_t childHandle = child["Handle"].as<uint64_t>();
					deserializedActor.AddChild(static_cast<UUID>(childHandle));
				}
			}

			auto transformComponent = actor["TransformComponent"];
			if (transformComponent)
			{
				// All Entities have a transform
				auto& transform = deserializedActor.GetComponent<TransformComponent>();
				transform.Translation = transformComponent["Translation"].as<Math::vec3>();
				transform.SetRotationEuler(transformComponent["Rotation"].as<Math::vec3>());
				transform.Scale = transformComponent["Scale"].as<Math::vec3>();
			}

			auto cameraComponent = actor["CameraComponent"];
			if (cameraComponent)
			{
				auto& cc = deserializedActor.AddComponent<CameraComponent>();

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
				if (cameraComponent["PostProcessingEnabled"])
					cc.PostProcessing.Enabled = cameraComponent["PostProcessingEnabled"].as<bool>();

				if (cc.PostProcessing.Enabled)
				{
					auto postProcessData = cameraComponent["PostProcessing"];
					
					{
						auto bloomData = postProcessData["Bloom"];
						cc.PostProcessing.Bloom.Threshold = bloomData["Threshold"].as<float>();
						cc.PostProcessing.Bloom.Knee = bloomData["Knee"].as<float>();
						cc.PostProcessing.Bloom.Intensity = bloomData["Intensity"].as<float>();
						cc.PostProcessing.Bloom.Enabled = bloomData["Enabled"].as<float>();
					}
				}
			}

			auto skyboxComponent = actor["SkyboxComponent"];
			if (skyboxComponent)
			{
				auto& skybox = deserializedActor.AddComponent<SkyboxComponent>();

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

			auto lightSourceComponent = actor["LightSourceComponent"];
			if (lightSourceComponent)
			{
				auto& lightComponent = deserializedActor.AddComponent<LightSourceComponent>();
				
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

			auto meshComponent = actor["MeshRendererComponent"];
			if (meshComponent)
			{
				if (meshComponent["MeshHandle"])
				{
					auto& meshRendererComponent = deserializedActor.AddComponent<MeshRendererComponent>();
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

			auto staticMeshComponent = actor["StaticMeshRendererComponent"];
			if (staticMeshComponent)
			{
				auto& staticMeshRendererComponent = deserializedActor.AddComponent<StaticMeshRendererComponent>();
				staticMeshRendererComponent.Type = Utils::MeshTypeFromString(staticMeshComponent["MeshType"].as<std::string>());

				if (staticMeshRendererComponent.Type != MeshType::Custom)
				{
					DefaultMesh::StaticMeshType defaultMesh = (DefaultMesh::StaticMeshType)staticMeshRendererComponent.Type;
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

								// Load material textures so it is ready to go for rendering
								auto material = AssetManager::GetAsset<Material>(materialHandle);
								AssetManager::GetAsset<Texture2D>(material->GetAlbedoMap());
								AssetManager::GetAsset<Texture2D>(material->GetNormalMap());
								AssetManager::GetAsset<Texture2D>(material->GetMetallicMap());
								AssetManager::GetAsset<Texture2D>(material->GetRoughnessMap());
								AssetManager::GetAsset<Texture2D>(material->GetEmissionMap());
								AssetManager::GetAsset<Texture2D>(material->GetAmbientOcclusionMap());

								materialTable->SetMaterial(submeshIndex++, materialHandle);
							}
						}
					}
				}
			}

			auto spriteComponent = actor["SpriteRendererComponent"];
			if (spriteComponent)
			{
				auto& spriteRendererComponent = deserializedActor.AddComponent<SpriteRendererComponent>();
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

			auto circleComponent = actor["CircleRendererComponent"];
			if (circleComponent)
			{
				auto& circleRendererComponent = deserializedActor.AddComponent<CircleRendererComponent>();

				if (circleComponent["Visible"])
					circleRendererComponent.Visible = circleComponent["Visible"].as<bool>();

				circleRendererComponent.Color = circleComponent["Color"].as<Math::vec4>();
				circleRendererComponent.Thickness = circleComponent["Thickness"].as<float>();
				circleRendererComponent.Fade = circleComponent["Fade"].as<float>();
			}

			auto particleEmitterComponent = actor["ParticleEmitterComponent"];
			if (particleEmitterComponent)
			{
				auto& pmc = deserializedActor.AddComponent<ParticleEmitterComponent>();

				if (particleEmitterComponent["EmitterHandle"])
				{
					AssetHandle emitterHandle = particleEmitterComponent["EmitterHandle"].as<uint64_t>();
					if (AssetManager::IsHandleValid(emitterHandle))
					{
						pmc.EmitterHandle = emitterHandle;
					}
				}

				if (particleEmitterComponent["IsActive"])
				{
					pmc.IsActive = particleEmitterComponent["IsActive"].as<bool>();
				}
				else
				{
					pmc.IsActive = false;
				}
			}

			auto textMeshComponent = actor["TextMeshComponent"];
			if (textMeshComponent)
			{
				auto& tmc = deserializedActor.AddComponent<TextMeshComponent>();

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
				if (textMeshComponent["BackgroundColor"])
					tmc.BackgroundColor = textMeshComponent["BackgroundColor"].as<Math::vec4>();
				tmc.Kerning = textMeshComponent["Kerning"].as<float>();
				tmc.LineSpacing = textMeshComponent["LineSpacing"].as<float>();
				tmc.MaxWidth = textMeshComponent["MaxWidth"].as<float>();
				tmc.TextHash = textMeshComponent["TextHash"].as<size_t>();
				tmc.TextString = textMeshComponent["TextString"].as<std::string>();
			}

			auto animationComponent = actor["AnimationComponent"];
			if (animationComponent)
			{
				if (!deserializedActor.HasComponent<MeshRendererComponent>())
				{
					VX_CONSOLE_LOG_WARN("Trying to add Animation Component without Mesh Renderer Component!");
					return;
				}

				// TODO fix animations to take in mesh asset handle
				/*auto& animation = deserializedActor.AddComponent<AnimationComponent>();
				SharedRef<Mesh> mesh = deserializedActor.GetComponent<MeshRendererComponent>().Mesh;
				std::string filepath = mesh->GetPath();
				animation.Animation = Animation::Create(filepath, mesh);*/
			}

			auto animatorComponent = actor["AnimatorComponent"];
			if (animatorComponent)
			{
				if (!deserializedActor.HasComponent<AnimationComponent>())
				{
					VX_CONSOLE_LOG_WARN("Trying to add Animator Component without Animation Component!");
					return;
				}

				auto& animator = deserializedActor.AddComponent<AnimatorComponent>();
				animator.Animator = Animator::Create(deserializedActor.GetComponent<AnimationComponent>().Animation);
			}

			auto audioSourceComponent = actor["AudioSourceComponent"];
			if (audioSourceComponent)
			{
				auto& asc = deserializedActor.AddComponent<AudioSourceComponent>();

				AssetHandle audioHandle = audioSourceComponent["AudioHandle"].as<uint64_t>();
				if (AssetManager::IsHandleValid(audioHandle))
				{
					asc.AudioHandle = audioHandle;
				}

				if (audioSourceComponent["PlayOnStart"])
				{
					asc.PlayOnStart = audioSourceComponent["PlayOnStart"].as<bool>();
				}
				else
				{
					asc.PlayOnStart = false;
				}

				if (audioSourceComponent["PlayOneShot"])
				{
					asc.PlayOneShot = audioSourceComponent["PlayOneShot"].as<bool>();
				}
				else
				{
					asc.PlayOneShot = false;
				}
			}

			auto audioListenerComponent = actor["AudioListenerComponent"];
			if (audioListenerComponent)
			{
				// TODO
			}

			auto rigidbodyComponent = actor["RigidbodyComponent"];
			if (rigidbodyComponent)
			{
				auto& rigidbody = deserializedActor.AddComponent<RigidBodyComponent>();

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

			auto characterControllerComponent = actor["CharacterControllerComponent"];
			if (characterControllerComponent)
			{
				auto& characterController = deserializedActor.AddComponent<CharacterControllerComponent>();

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

			auto fixedJointComponent = actor["FixedJointComponent"];
			if (fixedJointComponent)
			{
				auto& fixedJoint = deserializedActor.AddComponent<FixedJointComponent>();

				VX_DESERIALIZE_PROPERTY(ConnectedActor, uint64_t, fixedJoint.ConnectedActor, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(BreakForce, float, fixedJoint.BreakForce, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(BreakTorque, float, fixedJoint.BreakTorque, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(EnableCollision, bool, fixedJoint.EnableCollision, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(EnablePreProcessing, bool, fixedJoint.EnablePreProcessing, fixedJointComponent);
				VX_DESERIALIZE_PROPERTY(IsBreakable, bool, fixedJoint.IsBreakable, fixedJointComponent);
			}

			auto boxColliderComponent = actor["BoxColliderComponent"];
			if (boxColliderComponent)
			{
				auto& boxCollider = deserializedActor.AddComponent<BoxColliderComponent>();

				boxCollider.HalfSize = boxColliderComponent["HalfSize"].as<glm::vec3>();
				boxCollider.Offset = boxColliderComponent["Offset"].as<glm::vec3>();
				if (boxColliderComponent["IsTrigger"])
					boxCollider.IsTrigger = boxColliderComponent["IsTrigger"].as<bool>();
				if (boxColliderComponent["Visible"])
					boxCollider.Visible = boxColliderComponent["Visible"].as<bool>();

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

			auto sphereColliderComponent = actor["SphereColliderComponent"];
			if (sphereColliderComponent)
			{
				auto& sphereCollider = deserializedActor.AddComponent<SphereColliderComponent>();

				sphereCollider.Radius = sphereColliderComponent["Radius"].as<float>();
				sphereCollider.Offset = sphereColliderComponent["Offset"].as<Math::vec3>();
				sphereCollider.IsTrigger = sphereColliderComponent["IsTrigger"].as<bool>();
				if (sphereColliderComponent["Visible"])
					sphereCollider.Visible = sphereColliderComponent["Visible"].as<bool>();

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

			auto capsuleColliderComponent = actor["CapsuleColliderComponent"];
			if (capsuleColliderComponent)
			{
				auto& capsuleCollider = deserializedActor.AddComponent<CapsuleColliderComponent>();

				capsuleCollider.Radius = capsuleColliderComponent["Radius"].as<float>();
				capsuleCollider.Height = capsuleColliderComponent["Height"].as<float>();
				capsuleCollider.Offset = capsuleColliderComponent["Offset"].as<Math::vec3>();
				capsuleCollider.IsTrigger = capsuleColliderComponent["IsTrigger"].as<bool>();
				if (capsuleColliderComponent["Visible"])
					capsuleCollider.Visible = capsuleColliderComponent["Visible"].as<bool>();

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

			auto meshColliderComponent = actor["MeshColliderComponent"];
			if (meshColliderComponent)
			{
				auto& meshCollider = deserializedActor.AddComponent<MeshColliderComponent>();

				// TODO
			}

			auto rigidbody2DComponent = actor["Rigidbody2DComponent"];
			if (rigidbody2DComponent)
			{
				auto& rb2d = deserializedActor.AddComponent<RigidBody2DComponent>();

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

			auto boxCollider2DComponent = actor["BoxCollider2DComponent"];
			if (boxCollider2DComponent)
			{
				auto& bc2d = deserializedActor.AddComponent<BoxCollider2DComponent>();

				bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
				bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
				bc2d.Density = boxCollider2DComponent["Density"].as<float>();
				bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
				bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
				bc2d.RestitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
				if (boxCollider2DComponent["IsTrigger"])
					bc2d.IsTrigger = boxCollider2DComponent["IsTrigger"].as<bool>();
			}

			auto circleCollider2DComponent = actor["CircleCollider2DComponent"];
			if (circleCollider2DComponent)
			{
				auto& cc2d = deserializedActor.AddComponent<CircleCollider2DComponent>();

				cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
				cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
				cc2d.Density = circleCollider2DComponent["Density"].as<float>();
				cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
				cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
				cc2d.RestitutionThreshold = circleCollider2DComponent["RestitutionThreshold"].as<float>();
			}

			auto scriptComponent = actor["ScriptComponent"];
			if (scriptComponent)
			{
				auto& sc = deserializedActor.AddComponent<ScriptComponent>();
				sc.ClassName = scriptComponent["ClassName"].as<std::string>();

				if (ScriptEngine::ActorClassExists(sc.ClassName))
				{
					auto scriptFields = scriptComponent["ScriptFields"];

					if (scriptFields)
					{
						SharedReference<ScriptClass> actorClass = ScriptEngine::GetActorClass(sc.ClassName);

						if (actorClass)
						{
							const auto& fields = actorClass->GetFields();
							ScriptFieldMap& actorScriptFields = ScriptEngine::GetMutableScriptFieldMap(deserializedActor);

							for (auto scriptField : scriptFields)
							{
								std::string name = scriptField["Name"].as<std::string>();

								ScriptFieldInstance& fieldInstance = actorScriptFields[name];

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

}
