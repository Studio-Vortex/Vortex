#pragma once

#include "Sparky/Core/Math.h"
#include "Sparky/Core/UUID.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Renderer/Texture.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Audio/AudioListener.h"

namespace Sparky {

#pragma region Core Components

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
		std::string Tag = "";
		std::string Marker = "UnTagged";
		inline static std::vector<std::string> Markers = { "UnTagged", "Start", "Finish", "Player", "MainCamera" };

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

	struct TransformComponent
	{
		Math::vec3 Translation = Math::vec3(0.0f);
		Math::vec3 Rotation = Math::vec3(0.0f);
		Math::vec3 Scale = Math::vec3(1.0f);

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::vec3& translation)
			: Translation(translation) { }
		TransformComponent(const Math::vec3& translation, const Math::vec3& rotation, const Math::vec3& scale)
			: Translation(translation), Rotation(rotation), Scale(scale) { }

		inline Math::mat4 GetTransform() const
		{
			Math::mat4 rotation = Math::ToMat4(Math::Quaternion(Rotation));
			Math::mat4 result = Math::Translate(Translation) * rotation * Math::Scale(Scale);
			return result;
		}
	};

#pragma endregion

#pragma region Rendering Components

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	// Forward declaration
	class Skybox;

	struct SkyboxComponent
	{
		SharedRef<Skybox> Source = nullptr;

		SkyboxComponent() = default;
		SkyboxComponent(const SkyboxComponent&) = default;
	};

	// Forward declaration
	class LightSource;

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

	// Forward declaration
	class Model;

	struct MeshRendererComponent
	{
		enum class MeshType { Cube = 0, Sphere, Capsule, Cone, Cylinder, Plane, Torus, Custom };
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

	// Forward declaration
	class ParticleEmitter;

	struct ParticleEmitterComponent
	{
		SharedRef<ParticleEmitter> Emitter = nullptr;

		ParticleEmitterComponent() = default;
		ParticleEmitterComponent(const ParticleEmitterComponent&) = default;
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

	struct RigidBodyComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		bool ConstrainXAxis = false;
		bool ConstrainYAxis = false;
		bool ConstrainZAxis = false;

		RigidBodyComponent() = default;
		RigidBodyComponent(const RigidBodyComponent&) = default;
	};

	struct BoxColliderComponent
	{
		Math::vec3 Offset = Math::vec3(0.0f);
		Math::vec3 Size = Math::vec3(0.5f);

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;

		BoxColliderComponent() = default;
		BoxColliderComponent(const BoxColliderComponent&) = default;
	};

	struct SphereColliderComponent
	{
		Math::vec3 Offset = Math::vec3(0.0f);
		float Radius = 0.5f;

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		SphereColliderComponent() = default;
		SphereColliderComponent(const SphereColliderComponent&) = default;
	};

	struct CapsuleColliderComponent
	{
		Math::vec2 Offset = Math::vec2(0.0f);
		Math::vec3 Size = Math::vec3(0.5f);

		// TODO: Move into physics material in the future
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CapsuleColliderComponent() = default;
		CapsuleColliderComponent(const CapsuleColliderComponent&) = default;
	};

	struct StaticMeshColliderComponent
	{
		uint32_t TODO;

		StaticMeshColliderComponent() = default;
		StaticMeshColliderComponent(const StaticMeshColliderComponent&) = default;
	};

	struct RigidBody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		Math::vec2 Velocity = Math::vec2(0.0f);
		float Drag = 0.0f;

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

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward declaration
	class ScriptableEntity;

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
		ComponentGroup<TransformComponent,
		// Rendering
		CameraComponent, SkyboxComponent, LightSourceComponent, MeshRendererComponent,
		SpriteRendererComponent, CircleRendererComponent, ParticleEmitterComponent,
		// Audio
		AudioSourceComponent, AudioListenerComponent,
		// Physics
		RigidBodyComponent, BoxColliderComponent, SphereColliderComponent, CapsuleColliderComponent, StaticMeshColliderComponent,
		RigidBody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
		// Script
		ScriptComponent, NativeScriptComponent>;

}
