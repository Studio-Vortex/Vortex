#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Asset/Asset.h"

#include "Vortex/Scene/SceneCamera.h"

#include "Vortex/Renderer/Material.h"

#include "Vortex/Physics/3D/PhysXTypes.h"

namespace Vortex {

#pragma region Core Components

	class Prefab;

	struct VORTEX_API IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		IDComponent(UUID uuid)
			: ID(uuid) { }
	};

	struct VORTEX_API TagComponent
	{
		inline static std::vector<std::string> Markers = { "Untagged", "Start", "Finish", "Player", "MainCamera" };

		std::string Tag = "";
		std::string Marker = Markers[0];
		bool IsActive = true;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) { }

		inline static void AddMarker(const std::string& marker)
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

	struct VORTEX_API HierarchyComponent
	{
		UUID ParentUUID = 0;
		std::vector<UUID> Children;

		HierarchyComponent() = default;
		HierarchyComponent(const HierarchyComponent&) = default;
		HierarchyComponent(UUID parentUUID)
			: ParentUUID(parentUUID) { }
	};

	enum class Space { Local, World, };

	struct VORTEX_API TransformComponent
	{
	public:
		Math::vec3 Translation = Math::vec3(0.0f);
		Math::vec3 Scale = Math::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::vec3& translation)
			: Translation(translation) { }
		TransformComponent(const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) { }

		inline Math::mat4 GetTransform() const
		{
			return Math::Translate(Translation) * Math::ToMat4(Rotation) * Math::Scale(Scale);
		}

		inline void SetTransform(const Math::mat4& transform)
		{
			Math::vec3 skew;
			Math::vec4 perspective;
			Math::Decompose(transform, Scale, Rotation, Translation, skew, perspective);
			RotationEuler = Math::EulerAngles(Rotation);
		}

		inline Math::quaternion GetRotation() const
		{
			return Rotation;
		}

		inline void SetRotation(const Math::quaternion& rotation)
		{
			Rotation = rotation;
			RotationEuler = Math::EulerAngles(Rotation);
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

	private:
		Math::vec3 RotationEuler = Math::vec3(0.0f);
		Math::quaternion Rotation = Math::quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	};

	struct VORTEX_API PrefabComponent
	{
		AssetHandle PrefabAsset = 0;
		UUID PrefabUUID = 0;
		UUID EntityUUID = 0;

		PrefabComponent() = default;
		PrefabComponent(const PrefabComponent&) = default;
	};

#pragma endregion

#pragma region Rendering Components

	class Animator;
	class Animation;

	struct VORTEX_API CameraComponent
	{
		SceneCamera Camera;
		Math::vec3 ClearColor = Math::vec3((38.0f / 255.0f), (44.0f / 255.0f), (60.0f / 255.0f)); // Dark blue
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct VORTEX_API SkyboxComponent
	{
		AssetHandle Skybox = 0;
		float Rotation = 0.0f;
		float Intensity = 1.0f;

		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;
	};

	struct VORTEX_API LightSource2DComponent
	{
		Math::vec3 Color = Math::vec3(1.0f);
		float Intensity = 1.0f;

		bool Visible = true;

		LightSource2DComponent() = default;
		LightSource2DComponent(const LightSource2DComponent&) = default;
	};

	enum class LightType { Directional = 0, Point, Spot };

	struct VORTEX_API LightSourceComponent
	{
		LightType Type = LightType::Directional;

		Math::vec3 Radiance = Math::vec3(1.0f);
		float Intensity = 1.0f;

		// Spotlight Only
		float Cutoff = 12.5f;
		float OuterCutoff = 17.5f;

		// Shadow Settings
		float ShadowBias = 0.2f;
		bool CastShadows = true;
		bool SoftShadows = true;
		
		bool Visible = true;

		LightSourceComponent() = default;
		LightSourceComponent(const LightSourceComponent&) = default;
		LightSourceComponent(LightType type)
			: Type(type) { }
	};

	struct VORTEX_API MeshRendererComponent
	{
		AssetHandle Mesh = 0;
		SharedReference<MaterialTable> Materials = SharedReference<MaterialTable>::Create();
		bool Visible = true;

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
	};

	enum class MeshType { Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus, Custom };

	struct VORTEX_API StaticMeshRendererComponent
	{
		AssetHandle StaticMesh = 0;
		MeshType Type = MeshType::Cube;
		SharedReference<MaterialTable> Materials = SharedReference<MaterialTable>::Create();
		bool Visible = true;

		StaticMeshRendererComponent() = default;
		StaticMeshRendererComponent(const StaticMeshRendererComponent&) = default;
	};

	struct VORTEX_API SpriteRendererComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);
		AssetHandle Texture = 0;
		Math::vec2 TextureUV = Math::vec2(1.0f);
		bool Visible = true;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const Math::vec4& color)
			: SpriteColor(color) { }
	};

	struct VORTEX_API CircleRendererComponent
	{
		Math::vec4 Color = Math::vec4(1.0f);
		float Thickness = 1.0f;
		float Fade = 0.005f;
		bool Visible = true;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct VORTEX_API ParticleEmitterComponent
	{
		AssetHandle EmitterHandle = 0;

		ParticleEmitterComponent() = default;
		ParticleEmitterComponent(const ParticleEmitterComponent&) = default;
	};

	struct VORTEX_API AnimatorComponent
	{
		SharedRef<Animator> Animator = nullptr;

		AnimatorComponent() = default;
		AnimatorComponent(const AnimatorComponent&) = default;
	};

	struct VORTEX_API AnimationComponent
	{
		SharedRef<Animation> Animation = nullptr;

		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent&) = default;
	};

#pragma endregion

#pragma region UI Components

	struct VORTEX_API TextMeshComponent
	{
		AssetHandle FontAsset = 0;
		std::string TextString = "";
		size_t TextHash = 0;
		bool Visible = true;

		// Font
		Math::vec4 Color = Math::vec4(1.0f);
		Math::vec4 BgColor = Math::vec4(0.0f);
		float LineSpacing = 0.0f;
		float Kerning = 0.0f;

		// Layout
		float MaxWidth = 10.0f;

		TextMeshComponent() = default;
		TextMeshComponent(const TextMeshComponent&) = default;
	};

	struct VORTEX_API ButtonComponent
	{
		UUID UI_ID = 0;
		AssetHandle FontAsset = 0;
		std::string TextString = "";
		size_t TextHash = 0;

		// Font
		Math::vec4 TextColor = Math::vec4(1.0f);
		Math::vec4 BgColor = Math::vec4(0.8f, 0.8f, 0.8f, 1.0f);
		float LineSpacing = 0.0f;
		float Kerning = 0.0f;

		// Layout
		float MaxWidth = 10.0f;

		ButtonComponent() = default;
		ButtonComponent(const ButtonComponent&) = default;
	};

#pragma endregion

#pragma region Audio Components

	struct VORTEX_API AudioSourceComponent
	{
		AssetHandle AudioHandle = 0;

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};
	
	struct VORTEX_API AudioListenerComponent
	{
		AssetHandle ListenerHandle = 0;
		
		AudioListenerComponent() = default;
		AudioListenerComponent(const AudioListenerComponent&) = default;
	};

#pragma endregion

#pragma region Physics Components

	enum class RigidBodyType { None = -1, Static, Dynamic };

	struct VORTEX_API RigidBodyComponent
	{
		RigidBodyType Type = RigidBodyType::Static;
		uint32_t LayerID = 0;

		float Mass = 1.0f;
		Math::vec3 LinearVelocity = Math::vec3(0.0f);
		float MaxLinearVelocity = 100.0f;
		float LinearDrag = 0.01f;
		Math::vec3 AngularVelocity = Math::vec3(0.0f);
		float MaxAngularVelocity = 100.0f;
		float AngularDrag = 0.05f;
		bool DisableGravity = false;
		bool IsKinematic = false;

		CollisionDetectionType CollisionDetection = CollisionDetectionType::Discrete;

		uint8_t LockFlags = 0;

		void* RuntimeActor = nullptr;

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
	};

	struct VORTEX_API CharacterControllerComponent
	{
		NonWalkableMode NonWalkMode = NonWalkableMode::PreventClimbing;
		CapsuleClimbMode ClimbMode = CapsuleClimbMode::Constrained;

		float SpeedDown = 0.0f;
		float SlopeLimitDegrees = 45.0f;
		float StepOffset = 1.0f;
		float ContactOffset = 0.01f;
		uint32_t LayerID = 0;
		bool DisableGravity = false;

		void* RuntimeController = nullptr;

		CharacterControllerComponent() = default;
		CharacterControllerComponent(const CharacterControllerComponent&) = default;
	};

	struct VORTEX_API FixedJointComponent
	{
		UUID ConnectedEntity;

		bool IsBreakable = true;
		float BreakForce = 100.0f;
		float BreakTorque = 10.0f;

		bool EnableCollision = false;
		bool EnablePreProcessing = true;
	};

	struct VORTEX_API BoxColliderComponent
	{
		Math::vec3 HalfSize = Math::vec3(0.5f);
		Math::vec3 Offset = Math::vec3(0.0f);
		AssetHandle Material = 0;

		bool Visible = false;
		bool IsTrigger = false;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};

	struct VORTEX_API SphereColliderComponent
	{
		float Radius = 0.5f;
		Math::vec3 Offset = Math::vec3(0.0f);
		AssetHandle Material = 0;

		bool Visible = false;
		bool IsTrigger = false;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
	};

	struct VORTEX_API CapsuleColliderComponent
	{
		float Radius = 0.5f;
		float Height = 1.0f;
		Math::vec3 Offset = Math::vec3(0.0f);
		AssetHandle Material = 0;

		bool Visible = false;
		bool IsTrigger = false;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
	};

	struct VORTEX_API MeshColliderComponent
	{
		AssetHandle ColliderAsset = 0;
		uint32_t SubmeshIndex = 0;
		ECollisionComplexity CollisionComplexity = ECollisionComplexity::Default;
		AssetHandle Material = 0;

		bool Visible = false;
		bool IsTrigger = false;
		bool UseSharedShape = false;

		MeshColliderComponent() = default;
		MeshColliderComponent(const MeshColliderComponent&) = default;
	};

	enum class RigidBody2DType { Static = 0, Dynamic, Kinematic };

	struct VORTEX_API RigidBody2DComponent
	{
		RigidBody2DType Type = RigidBody2DType::Static;
		bool FixedRotation = false;

		Math::vec2 Velocity = Math::vec2(0.0f);
		float Drag = 0.0f;
		float AngularVelocity = 0.0f;
		float AngularDrag = 0.05f;
		float GravityScale = 1.0f;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		RigidBody2DComponent() = default;
		RigidBody2DComponent(const RigidBody2DComponent&) = default;
	};

	struct VORTEX_API BoxCollider2DComponent
	{
		Math::vec2 Offset = Math::vec2(0.0f);
		Math::vec2 Size = Math::vec2(0.5f);

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		bool Visible = false;
		bool IsTrigger = false;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct VORTEX_API CircleCollider2DComponent
	{
		Math::vec2 Offset = Math::vec2(0.0f);
		float Radius = 0.5f;

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		bool Visible = false;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

#pragma endregion

#pragma region AI Components

	struct VORTEX_API NavMeshAgentComponent
	{
		uint64_t Unknown = 0;

		NavMeshAgentComponent() = default;
		NavMeshAgentComponent(const NavMeshAgentComponent&) = default;
	};

#pragma endregion

#pragma region Script Components

	class ScriptableEntity;

	struct VORTEX_API ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	struct VORTEX_API NativeScriptComponent
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
	struct ComponentGroup { };

	using AllComponents =
		ComponentGroup<

		// Core
		HierarchyComponent, TransformComponent,

		// Rendering
		CameraComponent, SkyboxComponent, LightSourceComponent, MeshRendererComponent, StaticMeshRendererComponent,
		SpriteRendererComponent, CircleRendererComponent, ParticleEmitterComponent,
		TextMeshComponent, AnimatorComponent, AnimationComponent,

		// Audio
		AudioSourceComponent, AudioListenerComponent,

		// Physics
		RigidBodyComponent, CharacterControllerComponent, FixedJointComponent,
		BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, MeshColliderComponent,
		RigidBody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,

		// AI
		NavMeshAgentComponent,

		// Script
		ScriptComponent, NativeScriptComponent>;

}
