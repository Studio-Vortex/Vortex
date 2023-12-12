#include "vxpch.h"
#include "Vortex/Input/Input.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Events/KeyEvent.h"
#include "Vortex/Events/MouseEvent.h"

#include <GLFW/glfw3.h>

namespace Vortex {

	struct InputInternalData
	{
		Math::vec2 MouseScrollOffset;
		std::bitset<VX_MAX_KEYS> Keys;
		std::bitset<VX_MAX_KEYS> KeysChangedThisFrame;
		std::bitset<VX_MAX_MOUSE_BUTTONS> MouseButtons;
		std::bitset<VX_MAX_MOUSE_BUTTONS> MouseButtonsChangedThisFrame;

		SubModule Module;
	};

	static InputInternalData s_Data;

	void Input::Init()
	{
		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Input";
		moduleProps.APIVersion = Version(1, 2, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void Input::Shutdown()
	{
		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

	bool Input::IsKeyPressed(KeyCode keycode)
	{
		return s_Data.Keys.test((size_t)keycode) && KeyChangedThisFrame(keycode);
	}

	bool Input::IsKeyReleased(KeyCode keycode)
	{
		return !s_Data.Keys.test((size_t)keycode) && KeyChangedThisFrame(keycode);
	}

	bool Input::IsKeyDown(KeyCode keycode)
	{
		return s_Data.Keys.test((size_t)keycode);
	}

	bool Input::IsKeyUp(KeyCode keycode)
	{
		return !s_Data.Keys.test((size_t)keycode);
	}

	bool Input::IsMouseButtonPressed(MouseButton button)
	{
		return s_Data.MouseButtons.test((size_t)button) && MouseButtonChangedThisFrame(button);
	}

	bool Input::IsMouseButtonReleased(MouseButton button)
	{
		return !s_Data.MouseButtons.test((size_t)button) && MouseButtonChangedThisFrame(button);
	}

	bool Input::IsMouseButtonDown(MouseButton button)
	{
		return s_Data.MouseButtons.test((size_t)button);
	}

	bool Input::IsMouseButtonUp(MouseButton button)
	{
		return !s_Data.MouseButtons.test((size_t)button);
	}

	bool Input::IsGamepadButtonDown(GamepadButton gamepad)
	{
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
		{
			return state.buttons[(size_t)gamepad];
		}
	}

	bool Input::IsGamepadButtonUp(GamepadButton gamepad)
	{
		GLFWgamepadstate state;

		if (glfwGetGamepadState(GLFW_JOYSTICK_1, &state))
		{
			return !state.buttons[(size_t)gamepad];
		}
	}

	Math::vec2 Input::GetMouseScrollOffset()
	{
		Math::vec2 value = s_Data.MouseScrollOffset;

		// Reset the scroll offset
		s_Data.MouseScrollOffset = Math::vec2(0.0f);
		return value;
	}

	void Input::SetMouseScrollOffset(const Math::vec2& offset)
	{
		s_Data.MouseScrollOffset = offset;
	}

	float Input::GetGamepadAxis(GamepadAxis axis)
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

	void Input::OnEvent(const Event& e)
	{
		if (e.GetEventType() == EventType::MouseScrolled)
		{
			MouseScrolledEvent& scrolledEvent = (MouseScrolledEvent&)e;
			SetMouseScrollOffset({ scrolledEvent.GetXOffset(), scrolledEvent.GetYOffset() });
		}
	}

	void Input::UpdateKeyState(KeyCode key, int action)
	{
		if (action != GLFW_RELEASE)
		{
			if (!s_Data.Keys.test((size_t)key))
				s_Data.Keys.set((size_t)key, true);
		}
		else
		{
			s_Data.Keys.set((size_t)key, false);
		}

		s_Data.KeysChangedThisFrame.set((size_t)key, action != GLFW_REPEAT);
	}

	void Input::UpdateMouseButtonState(MouseButton button, int action)
	{
		if (action != GLFW_RELEASE)
		{
			if (!s_Data.MouseButtons.test((size_t)button))
				s_Data.MouseButtons.set((size_t)button, true);
		}
		else
		{
			s_Data.MouseButtons.set((size_t)button, false);
		}

		s_Data.MouseButtonsChangedThisFrame.set((size_t)button, action != GLFW_REPEAT);
	}

	bool Input::KeyChangedThisFrame(KeyCode key)
	{
		return s_Data.KeysChangedThisFrame.test((size_t)key);
	}

	bool Input::MouseButtonChangedThisFrame(MouseButton mousebutton)
	{
		return s_Data.MouseButtonsChangedThisFrame.test((size_t)mousebutton);
	}

	void Input::ResetChangesForNextFrame()
	{
		s_Data.KeysChangedThisFrame.reset();
		s_Data.MouseButtonsChangedThisFrame.reset();
	}

}
