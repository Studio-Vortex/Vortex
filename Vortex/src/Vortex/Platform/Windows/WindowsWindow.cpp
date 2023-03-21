#include "vxpch.h"
#include "WindowsWindow.h"

#include "Vortex/Core/Input/Input.h"
#include "Vortex/Events/WindowEvent.h"
#include "Vortex/Events/MouseEvent.h"
#include "Vortex/Events/KeyEvent.h"

#include "Vortex/Renderer/Renderer.h"

#include "Vortex/Platform/OpenGL/OpenGLContext.h"

#include <stb_image.h>

namespace Vortex {

	static constexpr const char* WINDOW_ICON_PATH = "Resources/Images/VortexLogo.png";

	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		VX_CONSOLE_LOG_ERROR("GLFW Error: ({}): {}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props)
	{
		VX_PROFILE_FUNCTION();

		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		VX_PROFILE_FUNCTION();

		Shutdown();
	}

	void WindowsWindow::Init(const WindowProperties& props)
	{
		VX_PROFILE_FUNCTION();

		m_Properties.Title = props.Title;
		m_Properties.Size = props.Size;
		m_Properties.Maximized = props.Maximized;
		m_Properties.VSync = props.VSync;
		m_Properties.Decorated = props.Decorated;

		VX_CONSOLE_LOG_INFO("Creating window '{}' {}", props.Title, props.Size);

		if (s_GLFWWindowCount == 0)
		{
			VX_PROFILE_SCOPE("glfwInit");

			int success = glfwInit();
			VX_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
		}

		{
			VX_PROFILE_SCOPE("glfwCreateWindow");

#ifdef VX_DEBUG
			RendererAPI::API api = Renderer::GetGraphicsAPI();
			switch (api)
			{
				case RendererAPI::API::OpenGL:
					glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
					break;
				case RendererAPI::API::Direct3D:
					break;
				case RendererAPI::API::Vulkan:
					break;
			}
#endif // VX_DEBUG

			RendererAPI::API rendererAPI = Renderer::GetGraphicsAPI();
			switch (rendererAPI)
			{
				case Vortex::RendererAPI::API::OpenGL:
					break;
				case Vortex::RendererAPI::API::Direct3D:
					glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
					break;
				case Vortex::RendererAPI::API::Vulkan:
					break;
			}

			if (!m_Properties.Decorated)
			{
				glfwWindowHint(GLFW_DECORATED, false);
			}

			if (m_Properties.Maximized)
			{
				glfwWindowHint(GLFW_MAXIMIZED, true);
			}

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
			const bool invalidKeyCode = action < (uint32_t)KeyCode::StartingKey || action >(uint32_t)KeyCode::MaxKeys;
			if (invalidKeyCode)
				return;

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			Input::UpdateKeyState((KeyCode)key, action);

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
					KeyPressedEvent event(static_cast<KeyCode>(key), 1);
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

			Input::UpdateMouseButtonState((MouseButton)button, action);

			switch (action)
			{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(static_cast<MouseButton>(button));
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(static_cast<MouseButton>(button));
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
				VX_CONSOLE_LOG_INFO("Joystick: {} Connected", jid);
			else if (event == GLFW_DISCONNECTED)
				VX_CONSOLE_LOG_INFO("Joystick: {} Disconnected", jid);
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
		VX_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapFrameBuffers();
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_Window, title.c_str());
		m_Properties.Title = title;
	}

	void WindowsWindow::SetSize(const Math::vec2& size)
	{
		glfwSetWindowSize(m_Window, (int)size.x, (int)size.y);
		m_Properties.Size = size;
	}

	void WindowsWindow::SetMaximized(bool maximized)
	{
		if (maximized)
			glfwMaximizeWindow(m_Window);
		else
			glfwRestoreWindow(m_Window);

		m_Properties.Maximized = maximized;
	}

    void WindowsWindow::SetDecorated(bool decorated)
    {
		glfwSetWindowAttrib(m_Window, GLFW_DECORATED, (int)decorated);
		m_Properties.Decorated = decorated;
    }

    void WindowsWindow::SetResizeable(bool resizeable)
    {
		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, (int)resizeable);
		m_Properties.Resizeable = resizeable;
    }

	void WindowsWindow::SetVSync(bool enabled)
	{
		VX_PROFILE_FUNCTION();
		glfwSwapInterval((int)enabled);
		m_Properties.VSync = enabled;
	}

	void WindowsWindow::CenterWindow() const
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

		int posX = (videoMode->width  >> 1) - ((int)m_Properties.Size.x >> 1);
		int posY = (videoMode->height >> 1) - ((int)m_Properties.Size.y >> 1);

		glfwSetWindowPos(m_Window, posX, posY);
	}

	void WindowsWindow::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		s_GLFWWindowCount--;

		if (s_GLFWWindowCount == 0)
			glfwTerminate();
	}

}
