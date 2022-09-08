#include "sppch.h"
#include "WindowsInput.h"

#include "Sparky/Core/Application.h"

#include <GLFW/glfw3.h>

namespace Sparky {

	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsKeyReleasedImpl(int keycode)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetKey(window, keycode);
		return state == GLFW_RELEASE;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonReleasedImpl(int button)
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		auto state = glfwGetMouseButton(window, button);
		return state == GLFW_RELEASE;
	}

	Math::vec2 WindowsInput::GetMousePositionImpl()
	{
		auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindowHandle());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos, (float)yPos };
	}

	float WindowsInput::GetMouseXImpl()
	{
		return GetMousePositionImpl().x;
	}

	float WindowsInput::GetMouseYImpl()
	{
		return GetMousePositionImpl().y;
	}

}