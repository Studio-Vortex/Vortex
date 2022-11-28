#pragma once

#include "Sparky/Core/UUID.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Audio/AudioListener.h"

namespace Sparky {

#pragma region Core Components

	// Forward declarations
	class Prefab;

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid)
			: ID(uuid) { }
	};

	struct TagComponent
	{
		inline static std::vector<std::string> Markers = { "UnTagged", "Start", "Finish", "Player", "MainCamera" };

		std::string Tag = "";
		std::string Marker = Markers[0];

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) { }

		inline void AddMarker(const std::string& marker)
		{
			Markers.push_back(marker);
			s_AddedMarkers.push_back(marker);
		}

		inline static void ResetAddedMarkers()
		{
			s_AddedMarkers.clear();
		}

		inline static const auto& GetAddedMarkers()
		{
			return s_AddedMarkers;
		}

	private:
		inline static std::vector<std::string> s_AddedMarkers;
	};

	struct HierarchyComponent
	{
		UUID ParentUUID = 0;
		std::vector<UUID> Children;

		HierarchyComponent() = default;
		HierarchyComponent(const HierarchyComponent&) = default;
		HierarchyComponent(UUID parentUUID)
			: ParentUUID(parentUUID) { }
	};

	struct TransformComponent
	{
		Math::vec3 Translation = Math::vec3(0.0f);
		Math::vec3 Scale = Math::vec3(1.0f);
	private:
		// These are private so that you are forced to set them via
		// SetRotation() or SetRotationEuler()
		// This avoids situation where one of them gets set and the other is forgotten.
		//
		// Why do we need both a quat and Euler angle representation for rotation?
		// Because Euler suffers from gimbal lock -> rotations should be stored as quaternions.
		//
		// BUT: quaternions are confusing, and humans like to work with Euler angles.
		// We cannot store just the quaternions and translate to/from Euler because the conversion
		// Euler -> quat -> Euler is not invariant.
		//
		// It's also sometimes useful to be able to store rotations > 360 degrees which
		// quats do not support.
		//
		// Accordingly, we store Euler for "editor" stuff that humans work with, 
		// and quats for everything else.  The two are maintained in-sync via the SetRotation()
		// methods.
		Math::vec3 RotationEuler = Math::vec3(0.0f);
		Math::quaternion Rotation = Math::quaternion(1.0f, 0.0f, 0.0f, 0.0f);

	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::vec3& translation)
			: Translation(translation) { }
		TransformComponent(const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) { }

		inline Math::mat4 GetTransform() const
		{
			return Math::Translate(Translation) * Math::ToMat4(Math::GetOrientation(RotationEuler)) * Math::Scale(Scale);
		}

		inline void SetTransform(const Math::mat4& transform)
		{
			Math::DecomposeTransform(transform, Translation, RotationEuler, Scale);
			//RotationEuler = Math::EulerAngles(Rotation);
		}

		inline Math::vec3 GetRotationEuler() const
		{
			return RotationEuler;
		}

		inline void SetRotationEuler(const Math::vec3& euler)
		{
			RotationEuler = euler;
			Rotation = Math::quaternion(RotationEuler);
		}

		inline Math::quaternion GetRotation() const
		{
			return Rotation;
		}

		inline void SetRotation(const Math::quaternion& quat)
		{
			Rotation = quat;
			RotationEuler = Math::EulerAngles(Rotation);
		}
	};

	struct PrefabComponent
	{
		SharedRef<Prefab> EntityPrefab = nullptr;
		UUID PrefabUUID = 0;
		UUID EntityUUID = 0;

		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent&) = default;
	};

#pragma endregion

#pragma region Rendering Components

	// Forward declarations
	class Skybox;
	class LightSource;
	class Model;
	class ParticleEmitter;
	class Font;

	struct CameraComponent
	{
		SceneCamera Camera;
		Math::vec3 ClearColor = Math::vec3((38.0f / 255.0f), (44.0f / 255.0f), (60.0f / 255.0f)); // Dark blue
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct SkyboxComponent
	{
		SharedRef<Skybox> Source = nullptr;

		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;
	};

	struct LightSourceComponent
	{
		enum class LightType { Directional = 0, Point, Spot };
		LightType Type = LightType::Directional;
		SharedRef<LightSource> Source = nullptr;

		LightSourceComponent() = default;
		LightSourceComponent(const LightSourceComponent&) = default;
		LightSourceComponent(LightType type, SharedRef<LightSource> source)
			: Type(type), Source(source) { }
	};

	enum class MeshType { Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus, Custom };

	struct MeshRendererComponent
	{
		MeshType Type = MeshType::Cube;
		SharedRef<Model> Mesh = nullptr;
		Math::vec2 Scale = Math::vec2(1.0f);
		bool Reflective = false;
		bool Refractive = false;

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);
		SharedRef<Texture2D> Texture = nullptr;
		Math::vec2 Scale = Math::vec2(1.0f);

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Math::vec4& color)
			: SpriteColor(color) { }
	};

	struct CircleRendererComponent
	{
		Math::vec4 Color = Math::vec4(1.0f);
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct ParticleEmitterComponent
	{
		SharedRef<ParticleEmitter> Emitter = nullptr;

		ParticleEmitterComponent() = default;
		ParticleEmitterComponent(const ParticleEmitterComponent&) = default;
	};

	struct TextMeshComponent
	{
		SharedRef<Font> FontAsset = nullptr;
		std::string TextString = "";
		size_t TextHash = 0;

		// Font
		Math::vec4 Color = Math::vec4(1.0f);
		float LineSpacing = 0.0f;
		float Kerning = 0.0f;

		// Layout
		float MaxWidth = 10.0f;

		TextMeshComponent() = default;
		TextMeshComponent(const TextMeshComponent&) = default;
	};

#pragma endregion

#pragma region Audio Components

	struct AudioSourceComponent
	{
		SharedRef<AudioSource> Source = nullptr;

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};
	
	struct AudioListenerComponent
	{
		SharedRef<AudioListener> Listener = nullptr;
		
		AudioListenerComponent() = default;
		AudioListenerComponent(const AudioListenerComponent&) = default;
	};

#pragma endregion

#pragma region Physics Components

	enum class RigidBodyType { None = -1, Static, Dynamic };

	struct RigidBodyComponent
	{
		RigidBodyType Type = RigidBodyType::Static;

		uint32_t LayerID = 0;

		float Mass = 1.0f;
		Math::vec3 LinearVelocity = Math::vec3(0.0f);
		float LinearDrag = 0.01f;
		Math::vec3 AngularVelocity = Math::vec3(0.0f);
		float AngularDrag = 0.05f;
		bool DisableGravity = false;
		bool IsKinematic = false;

		enum class CollisionDetectionType : uint32_t
		{
			Discrete,
			Continuous,
			ContinuousSpeculative
		};

		CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;

		bool LockPositionX = false;
		bool LockPositionY = false;
		bool LockPositionZ = false;
		bool LockRotationX = false;
		bool LockRotationY = false;
		bool LockRotationZ = false;

		void* RuntimeActor = nullptr;

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
	};

	struct CharacterControllerComponent
	{
		float SpeedDown = 0.0f;
		float SlopeLimitDegrees = 45.0f;
		float StepOffset = 0.0f;
		uint32_t LayerID = 0;
		bool DisableGravity = false;

		void* RuntimeController = nullptr;

		CharacterControllerComponent() = default;
		CharacterControllerComponent(const CharacterControllerComponent&) = default;
	};

	struct PhysicsMaterialComponent
	{
		float StaticFriction = 1.0F;
		float DynamicFriction = 1.0F;
		float Bounciness = 1.0F;

		PhysicsMaterialComponent() = default;
		PhysicsMaterialComponent(const PhysicsMaterialComponent&) = default;
	};

	struct BoxColliderComponent
	{
		Math::vec3 HalfSize = Math::vec3(0.5f);
		Math::vec3 Offset = Math::vec3(0.0f);
		bool IsTrigger = false;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};

	struct SphereColliderComponent
	{
		float Radius = 0.5f;
		Math::vec3 Offset = Math::vec3(0.0f);
		bool IsTrigger = false;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
	};

	struct CapsuleColliderComponent
	{
		float Radius = 0.5f;
		float Height = 1.0f;
		Math::vec3 Offset = Math::vec3(0.0f);
		bool IsTrigger = false;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
	};

	struct StaticMeshColliderComponent
	{
		uint32_t TODO;

		StaticMeshColliderComponent() = default;
		StaticMeshColliderComponent(const StaticMeshColliderComponent&) = default;
	};

	enum class RigidBody2DType { Static = 0, Dynamic, Kinematic };

	struct RigidBody2DComponent
	{
		RigidBody2DType Type = RigidBody2DType::Static;
		bool FixedRotation = false;

		Math::vec2 Velocity = Math::vec2(0.0f);
		float Drag = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		Math::vec2 Offset = Math::vec2(0.0f);
		Math::vec2 Size = Math::vec2(0.5f);

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		bool IsTrigger = false;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		Math::vec2 Offset = Math::vec2(0.0f);
		float Radius = 0.5f;

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

#pragma endregion

#pragma region Script Components

	// Forward declaration
	class ScriptableEntity;

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)() = nullptr;
		void (*DestroyInstanceScript)(NativeScriptComponent*) = nullptr;

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyInstanceScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

#pragma endregion

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<
		// Core
		HierarchyComponent, TransformComponent,
		// Rendering
		CameraComponent, SkyboxComponent, LightSourceComponent, MeshRendererComponent,
		SpriteRendererComponent, CircleRendererComponent, ParticleEmitterComponent, TextMeshComponent,
		// Audio
		AudioSourceComponent, AudioListenerComponent,
		// Physics
		RigidBodyComponent, CharacterControllerComponent, PhysicsMaterialComponent, BoxColliderComponent,
		SphereColliderComponent, CapsuleColliderComponent, StaticMeshColliderComponent,
		RigidBody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
		// Script
		ScriptComponent, NativeScriptComponent>;

}
