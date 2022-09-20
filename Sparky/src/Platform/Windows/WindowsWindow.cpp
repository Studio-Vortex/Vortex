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

		m_Data.Title = props.Title;
		m_Data.Size = props.Size;
		m_Data.StartMaximized = props.StartMaximized;
		m_Data.VSync = props.VSync;
		m_Data.Decorated = props.Decorated;

		SP_CORE_INFO("Creating window named '{}' with size: {}", props.Title, props.Size);

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

			if (m_Data.StartMaximized)
				glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
			else
				glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);

			if (m_Data.Decorated)
				glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
			else
				glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

			m_Window = glfwCreateWindow((int)m_Data.Size.x, (int)m_Data.Size.y, m_Data.Title.c_str(), nullptr, nullptr);
			s_GLFWWindowCount++;
		}

		m_Context = GraphicsContext::Create(m_Window);
		m_Context->Init();

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(m_Data.VSync);

		// Set GLFW Callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			data.Size = Math::vec2(width, height);

			WindowResizeEvent event(width, height);
			data.EventCallback(event);
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

    void WindowsWindow::SetMaximized(bool maximized) const
    {
		if (maximized)
			glfwMaximizeWindow(m_Window);
		else
			glfwRestoreWindow(m_Window);
    }

    void WindowsWindow::SetVSync(bool enabled)
	{
		SP_PROFILE_FUNCTION();
		glfwSwapInterval((int)enabled);
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
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