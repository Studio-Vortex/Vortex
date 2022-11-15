#include "sppch.h"
#include "WindowsWindow.h"

#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Events/MouseEvent.h"
#include "Sparky/Events/KeyEvent.h"

#include "Sparky/Core/Input.h"
#include "Sparky/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLContext.h"

#include <stb_image.h>

namespace Sparky {

	static constexpr const char* WINDOW_ICON_PATH = "Resources/Images/SparkyLogo.jpg";

	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		SP_CORE_ERROR("GLFW Error: ({}): {}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		SP_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		SP_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProperties& props)
	{
		SP_PROFILE_FUNCTION();

		m_Properties.Title = props.Title;
		m_Properties.Size = props.Size;
		m_Properties.Maximized = props.Maximized;
		m_Properties.VSync = props.VSync;
		m_Properties.Decorated = props.Decorated;

		SP_CORE_INFO("Creating window named '{}' with size: {}", props.Title, props.Size);

#ifdef SP_PLATFORM_WINDOWS
		SetConsoleTitleA(props.Title.c_str());
#endif // SP_PLATFORM_WINDOWS

		if (s_GLFWWindowCount == 0)
		{
			SP_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
			SP_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			SP_PROFILE_SCOPE("glfwCreateWindow");

#ifdef SP_DEBUG
			if (Renderer::GetGraphicsAPI() == RendererAPI::API::OpenGL)
				glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // SP_DEBUG

			if (m_Properties.Maximized)
				glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
			else
				glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

			if (m_Properties.Decorated)
				glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			else
				glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

			m_Window = glfwCreateWindow((int)m_Properties.Size.x, (int)m_Properties.Size.y, m_Properties.Title.c_str(), nullptr, nullptr);

			// Set the correct window height if maximized
			if (m_Properties.Maximized)
			{
				int maximzedWidth, maximizedHeight;
				glfwGetWindowSize(m_Window, &maximzedWidth, &maximizedHeight);
				m_Properties.Size = Math::vec2((float)maximzedWidth, (float)maximizedHeight);
			}

			s_GLFWWindowCount++;
		}

		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Properties);
		SetVSync(m_Properties.VSync);

		// Set GLFW Callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Size = Math::vec2((float)width, (float)height);

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
		});

		glfwSetWindowPosCallback(m_Window, [](GLFWwindow* window, int xpos, int ypos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Position = Math::vec2{ (float)xpos, (float)ypos };

			// TODO potentially a window moved event
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(static_cast<KeyCode>(key));
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(static_cast<KeyCode>(key), true);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event(static_cast<KeyCode>(keycode));
			data.EventCallback(event);
		});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<MouseCode>(button));
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			data.EventCallback(event);
		});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			MouseMovedEvent event((float)xPos, (float)yPos);
			data.EventCallback(event);
		});

		glfwSetJoystickCallback([](int jid, int event)
		{
			if (event == GLFW_CONNECTED)
				SP_CORE_INFO("Joystick: {} Connected", jid);
			else if (event == GLFW_DISCONNECTED)
				SP_CORE_INFO("Joystick: {} Disconnected", jid);
		});

		LoadWindowIcon();
	}

	void WindowsWindow::LoadWindowIcon()
	{
		int width, height, nrChannels;
		stbi_uc* textureData = stbi_load(WINDOW_ICON_PATH, &width, &height, &nrChannels, STBI_rgb_alpha);

		GLFWimage image{};
		image.width = width;
		image.height = height;
		image.pixels = textureData;
		glfwSetWindowIcon(m_Window, 1, &image);
		stbi_image_free(textureData);
	}

	void WindowsWindow::OnUpdate()
	{
		SP_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapFrameBuffers();
	}

	void WindowsWindow::SetMaximized(bool maximized)
	{
		auto getWindowSizeFunc = [&]() {
			int width;
			int height;
			glfwGetWindowSize(m_Window, &width, &height);
			m_Properties.Size = Math::vec2((float)width, (float)height);
		};

		m_Properties.Maximized = maximized;

		if (maximized)
		{
			glfwMaximizeWindow(m_Window);
			getWindowSizeFunc();
		}
		else
		{
			glfwRestoreWindow(m_Window);
			getWindowSizeFunc();
		}
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		m_Properties.Title = title;
		glfwSetWindowTitle(m_Window, title.c_str());
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		SP_PROFILE_FUNCTION();
		glfwSwapInterval((int)enabled);
		m_Properties.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Properties.VSync;
	}

	void WindowsWindow::CenterWindow() const
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

		int posX = (videoMode->width  >> 1) - ((int)m_Properties.Size.x >> 1);
		int posY = (videoMode->height >> 1) - ((int)m_Properties.Size.y >> 1);

		glfwSetWindowPos(m_Window, posX, posY);
	}

	void WindowsWindow::SetCursorPosition(uint32_t mouseCursorX, uint32_t mouseCursorY) const
	{
		glfwSetCursorPos(m_Window, (double)mouseCursorX, (double)mouseCursorY);
	}

	void WindowsWindow::ShowMouseCursor(bool enabled, bool rawInputMode) const
	{
		if (enabled)
		{
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
		}
		else
		{
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			if (rawInputMode)
				glfwSetInputMode(m_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
	}

	void WindowsWindow::Shutdown()
	{
		SP_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		s_GLFWWindowCount--;

		if (s_GLFWWindowCount == 0)
			glfwTerminate();
	}

}
