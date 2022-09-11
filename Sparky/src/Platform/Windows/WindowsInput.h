#pragma once

#include "Sparky/Core/Base.h"
#include "Sparky/Core/Input.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(int keycode) override;
		bool IsKeyReleasedImpl(int keycode) override;
		bool IsMouseButtonPressedImpl(int button) override;
		bool IsMouseButtonReleasedImpl(int button) override;
		Math::vec2 GetMousePositionImpl() override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};

}