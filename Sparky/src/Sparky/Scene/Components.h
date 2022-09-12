#pragma once

#include "Sparky/Core/Math.h"

namespace Sparky {

	struct TransformComponent
	{
		Math::mat4 Transform{ 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const Math::mat4& transform)
			: Transform(transform) {}

		operator Math::mat4& () { return Transform; }
		operator const Math::mat4& () const { return Transform; }
	};

	struct Sprite2DComponent
	{
		Math::vec4 SpriteColor = Math::vec4(1.0f);

		Sprite2DComponent() = default;
		Sprite2DComponent(const Sprite2DComponent&) = default;
		Sprite2DComponent(const Math::vec4& color)
			: SpriteColor(color) {}
	};

}