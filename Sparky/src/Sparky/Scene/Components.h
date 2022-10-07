#pragma once

#include "Sparky/Core/Math.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Core/UUID.h"
#include "Sparky/Renderer/Texture.h"

namespace Sparky {

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
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) { }
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

		Math::mat4 GetTransform() const
		{
			Math::mat4 rotation = Math::ToMat4(Math::Quaternion(Rotation));

			return Math::Translate(Translation) * rotation * Math::Scale(Scale);
		}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct SpriteRendererComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);
		SharedRef<Texture2D> Texture = nullptr;
		float Scale = 1.0f;

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

	// Physics components

	struct RigidBody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

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

	// Script components

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
		void (* DestroyInstanceScript)(NativeScriptComponent*) = nullptr;

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyInstanceScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<TransformComponent,
		CameraComponent, SpriteRendererComponent, CircleRendererComponent,
		RigidBody2DComponent, BoxCollider2DComponent, CircleCollider2DComponent,
		ScriptComponent, NativeScriptComponent>;

}
