#pragma once

#include "Sparky/Core.h"

#include "Sparky/Input.h"

namespace Sparky {

	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(int keycode) override;
		bool IsKeyReleasedImpl(int keycode) override;
		bool IsMouseButtonPressedImpl(int button) override;
		bool IsMouseButtonReleasedImpl(int button) override;
		std::pair<float, float> GetMousePositionImpl() override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;

	public:
	private:
	};

}