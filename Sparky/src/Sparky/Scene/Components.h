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
		Math::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::mat4& transform)
			: Transform(transform) { }

		operator Math::mat4& () { return Transform; }
		operator const Math::mat4& () const { return Transform; }
	};

	struct Sprite2DComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);

		Sprite2DComponent() = default;
		Sprite2DComponent(const Sprite2DComponent&) = default;
		Sprite2DComponent(const Math::vec4& color)
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

		std::function<void()> InstantiateFunction;
		std::function<void()> DestroyInstanceFunction;

		std::function<void(ScriptableEntity*)> OnCreateFunction;
		std::function<void(ScriptableEntity*, TimeStep)> OnUpdateFunction;
		std::function<void(ScriptableEntity*)> OnDestroyFunction;

		template <typename T>
		void Bind()
		{
			InstantiateFunction = [&]() { Instance = new T(); };
			DestroyInstanceFunction = [&]() { delete (T*)Instance; Instance = nullptr; };

			OnCreateFunction = [](ScriptableEntity* instance) { ((T*)instance)->OnCreate(); };
			OnUpdateFunction = [](ScriptableEntity* instance, TimeStep delta) { ((T*)instance)->OnUpdate(delta); };
			OnDestroyFunction = [](ScriptableEntity* instance) { ((T*)instance)->OnDestroy(); }; 
		}
	};

}