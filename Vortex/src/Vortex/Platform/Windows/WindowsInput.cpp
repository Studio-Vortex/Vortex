#include "vxpch.h"
#include "Vortex/Core/Input.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Events/MouseEvent.h"

#include <GLFW/glfw3.h>

namespace Vortex {

	static Math::vec2 s_MouseScrollOffset(0.0f);
	static std::bitset<VX_MAX_KEYS> s_Keys{};
	static std::bitset<VX_MAX_KEYS> s_KeysChangedThisFrame{};
	static std::bitset<VX_MAX_MOUSE_BUTTONS> s_MouseButtons{};
	static std::bitset<VX_MAX_MOUSE_BUTTONS> s_MouseButtonsChangedThisFrame{};

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		return s_Keys.test((size_t)keycode) && KeyChangedThisFrame(keycode);
	}

	bool Input::IsKeyReleased(KeyCode keycode)
	{
		return !s_Keys.test((size_t)keycode) && KeyChangedThisFrame(keycode);
	}

	bool Input::IsKeyDown(KeyCode keycode)
	{
		return s_Keys.test((size_t)keycode);
	}

	bool Input::IsKeyUp(KeyCode keycode)
	{
		return !s_Keys.test((size_t)keycode);
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return s_MouseButtons.test((size_t)button) && MouseButtonChangedThisFrame(button);
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return !s_MouseButtons.test((size_t)button) && MouseButtonChangedThisFrame(button);
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return s_MouseButtons.test((size_t)button);
	}

	bool Input::IsMouseButtonUp(MouseButton button)
	{
		return !s_MouseButtons.test((size_t)button);
	}

	bool Input::IsGamepadButtonDown(Gamepad gamepad)
	{
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
		{
			return state.buttons[(size_t)gamepad];
		}
	}

	bool Input::IsGamepadButtonUp(Gamepad gamepad)
	{
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
		{
			return !state.buttons[(size_t)gamepad];
		}
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
		{
			return state.axes[(size_t)axis];
		}
	}

	Math::vec2 Input::GetMousePosition()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
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

	void Input::SetMousePosition(const Math::vec2& position)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		glfwSetCursorPos(window, position.x, position.y);
	}

	CursorMode Input::GetCursorMode()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		return static_cast<CursorMode>(glfwGetInputMode(window, GLFW_CURSOR) - GLFW_CURSOR_NORMAL);
	}

	void Input::SetCursorMode(CursorMode cursorMode)
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL + (int)cursorMode);
	}

	void Input::UpdateMouseScrollOffset(const Event& event)
	{
		if (event.GetEventType() == EventType::MouseScrolled)
		{
			MouseScrolledEvent& scrolledEvent = (MouseScrolledEvent&)event;
			SetMouseScrollOffset({ scrolledEvent.GetXOffset(), scrolledEvent.GetYOffset() });
		}
	}

	void Input::UpdateKeyState(KeyCode key, int action)
	{
		if (action != GLFW_RELEASE)
		{
			if (!s_Keys.test((size_t)key))
				s_Keys.set((size_t)key, true);
		}
		else
		{
			s_Keys.set((size_t)key, false);
		}

		s_KeysChangedThisFrame.set((size_t)key, action != GLFW_REPEAT);
	}

	void Input::UpdateMouseButtonState(MouseButton button, int action)
	{
		if (action != GLFW_RELEASE)
		{
			if (!s_MouseButtons.test((size_t)button))
				s_MouseButtons.set((size_t)button, true);
		}
		else
		{
			s_MouseButtons.set((size_t)button, false);
		}

		s_MouseButtonsChangedThisFrame.set((size_t)button, action != GLFW_REPEAT);
	}

	bool Input::KeyChangedThisFrame(KeyCode key)
	{
		return s_KeysChangedThisFrame.test((size_t)key);
	}

	bool Input::MouseButtonChangedThisFrame(MouseButton mousebutton)
	{
		return s_MouseButtonsChangedThisFrame.test((size_t)mousebutton);
	}

	void Input::ResetChangesForNextFrame()
	{
		s_KeysChangedThisFrame.reset();
		s_MouseButtonsChangedThisFrame.reset();
	}

}
