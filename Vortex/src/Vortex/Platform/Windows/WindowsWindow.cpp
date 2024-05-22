#include "vxpch.h"
#include "WindowsWindow.h"

#include "Vortex/Input/Input.h"

#include "Vortex/Events/WindowEvent.h"
#include "Vortex/Events/MouseEvent.h"
#include "Vortex/Events/KeyEvent.h"

#include "Vortex/Renderer/Renderer.h"

#include "Vortex/Utils/FileSystem.h"

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

			switch (Renderer::GetGraphicsAPI())
			{
				case RendererAPI::API::OpenGL:
				{
#ifdef VX_DEBUG
					glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif // VX_DEBUG
					glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
					glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
					break;
				}
				case RendererAPI::API::Direct3D:
				{
					glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
					break;
				}
				case RendererAPI::API::Vulkan:
				{
					break;
				}
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

			// Set the correct window size if maximized
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

			WindowMoveEvent event(xpos, ypos);
			// TODO figure out why this crashes
			//data.EventCallback(event);
		});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			const bool invalidKeyCode = key < (int32_t)KeyCode::FirstKey || key > (int32_t)KeyCode::MaxKeys;
			if (invalidKeyCode)
			{
				return;
			}

			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			Input::UpdateKeyState((KeyCode)key, action);

			switch (action)
			{
				case GLFW_PRESS:
				{
					KeyPressedEvent event((KeyCode)key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event((KeyCode)key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event((KeyCode)key, 1);
					data.EventCallback(event);
					break;
				}
			}
		});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

			KeyTypedEvent event((KeyCode)keycode);
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
					MouseButtonPressedEvent event((MouseButton)button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event((MouseButton)button);
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

		glfwSetJoystickCallback([](int jid, int action)
		{
			if (action == GLFW_CONNECTED)
			{
				VX_CONSOLE_LOG_INFO("Joystick: {} Connected", jid);
			}
			else if (action == GLFW_DISCONNECTED)
			{
				VX_CONSOLE_LOG_INFO("Joystick: {} Disconnected", jid);
			}
		});

		glfwSetDropCallback(m_Window, [](GLFWwindow* window, int pathCount, const char* paths[])
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			
			std::vector<std::filesystem::path> filepaths(pathCount);
			for (uint32_t i = 0; i < pathCount; i++)
			{
				filepaths[i] = Fs::Path(paths[i]);
			}

			WindowDragDropEvent event(std::move(filepaths));
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
		VX_PROFILE_FUNCTION();

		glfwPollEvents();
		m_Context->SwapFrameBuffers();
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		m_Properties.Title = title;
		glfwSetWindowTitle(m_Window, m_Properties.Title.c_str());
	}

	void WindowsWindow::SetSize(const Math::vec2& size)
	{
		m_Properties.Size = size;
		glfwSetWindowSize(m_Window, (int)m_Properties.Size.x, (int)m_Properties.Size.y);
	}

	void WindowsWindow::SetMaximized(bool maximized)
	{
		m_Properties.Maximized = maximized;

		if (m_Properties.Maximized)
		{
			glfwMaximizeWindow(m_Window);
		}
		else
		{
			glfwRestoreWindow(m_Window);
		}
	}

    void WindowsWindow::SetDecorated(bool decorated)
    {
		m_Properties.Decorated = decorated;
		glfwSetWindowAttrib(m_Window, GLFW_DECORATED, (int)m_Properties.Decorated);
    }

    void WindowsWindow::SetResizeable(bool resizeable)
    {
		m_Properties.Resizeable = resizeable;
		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, (int)m_Properties.Resizeable);
    }

	void WindowsWindow::SetVSync(bool enabled)
	{
		m_Properties.VSync = enabled;
		glfwSwapInterval((int)m_Properties.VSync);
	}

	void WindowsWindow::CenterWindow() const
	{
		GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

		const int centerPosX = (videoMode->width  >> 1) - ((int)m_Properties.Size.x >> 1);
		const int centerPosY = (videoMode->height >> 1) - ((int)m_Properties.Size.y >> 1);

		glfwSetWindowPos(m_Window, centerPosX, centerPosY);
	}

	void WindowsWindow::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		glfwDestroyWindow(m_Window);
		s_GLFWWindowCount--;

		if (s_GLFWWindowCount == 0)
		{
			glfwTerminate();
		}
	}

}
