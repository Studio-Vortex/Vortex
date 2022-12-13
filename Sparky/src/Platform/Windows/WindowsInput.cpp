#include "sppch.h"
#include "Sparky/Core/Input.h"

#include "Sparky/Core/Application.h"
#include "Sparky/Events/MouseEvent.h"

#include <GLFW/glfw3.h>

namespace Sparky {

	static Math::vec2 s_MouseScrollOffset(0.0f);

    void Input::Update(const Event& event)
    {
		if (event.GetEventType() == EventType::MouseScrolled)
		{
			MouseScrolledEvent& scrolledEvent = (MouseScrolledEvent&)event;
			SetMouseScrollOffset({ scrolledEvent.GetXOffset(), scrolledEvent.GetYOffset() });
		}
    }

    bool Input::IsKeyPressed(KeyCode keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_PRESS;
	}

	bool Input::IsKeyReleased(KeyCode keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
		return state == GLFW_RELEASE;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonReleased(MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_RELEASE;
	}

    bool Input::IsGamepadButtonPressed(Gamepad gamepad)
    {
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
			return state.buttons[static_cast<int32_t>(gamepad)] == true;
    }

    bool Input::IsGamepadButtonReleased(Gamepad gamepad)
    {
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
			return state.buttons[static_cast<int32_t>(gamepad)] == false;
    }

	Math::vec2 Input::GetMouseScrollOffset()
	{
		Math::vec2 value = s_MouseScrollOffset;

		// Reset the scroll offset
		s_MouseScrollOffset = Math::vec2(0.0f);
		return value;
	}

	void Input::SetMouseScrollOffset(const Math::vec2& offset)
	{
		s_MouseScrollOffset = offset;
	}

	float Input::GetGamepadAxis(Gamepad axis)
	{
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
			return state.axes[static_cast<int32_t>(axis)];
	}

	Math::vec2 Input::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos, (float)yPos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

}
