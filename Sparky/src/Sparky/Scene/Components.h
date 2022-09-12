#pragma once

#include "Sparky/Core/Math.h"
#include "Sparky/Renderer/Camera.h"

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
		Sparky::Camera Camera;
		bool Primary = true;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const Math::mat4& projection)
			: Camera(projection) { }
	};

}