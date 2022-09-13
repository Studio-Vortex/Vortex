#pragma once

#include "Sparky/Core/Math.h"
#include "Sparky/Scene/SceneCamera.h"
#include "Sparky/Scene/ScriptableEntity.h"

namespace Sparky {

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
			Math::mat4 rotation = Math::Rotate(Rotation.x, { 1.0f, 0.0f, 0.0f })
				* Math::Rotate(Rotation.y, { 0.0f, 1.0f, 0.0f })
				* Math::Rotate(Rotation.z, { 0.0f, 0.0f, 1.0f });

			return Math::Translate(Translation) * rotation * Math::Scale(Scale);
		}
	};

	struct SpriteComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);

		SpriteComponent() = default;
		SpriteComponent(const SpriteComponent&) = default;
		SpriteComponent(const Math::vec4& color)
			: SpriteColor(color) { }
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity* (*InstantiateScript)();
		void (* DestroyInstanceScript)(NativeScriptComponent*);

		template <typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyInstanceScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

}