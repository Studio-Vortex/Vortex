#include "sppch.h"
#include "Application.h"

#include "Sparky/Renderer/Renderer.h"

#include "Sparky/Scripting/ScriptEngine.h"
#include "Sparky/Audio/AudioEngine.h"

#include "Sparky/Utils/PlatformUtils.h"
#include "Sparky/Events/KeyEvent.h"

extern bool g_ApplicationRunning;

namespace Sparky {

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationProperties& props)
		: m_Properties(props)
	{
		SP_PROFILE_FUNCTION();

		SP_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		// Set working directory here
		if (!m_Properties.WorkingDirectory.empty())
			std::filesystem::current_path(m_Properties.WorkingDirectory);

		WindowProperties windowProps;
		windowProps.Size = { m_Properties.WindowWidth, m_Properties.WindowHeight };
		windowProps.Title = m_Properties.Name;
		windowProps.Maximized = m_Properties.MaximizeWindow;
		windowProps.VSync = m_Properties.VSync;
		windowProps.Decorated = m_Properties.WindowDecorated;

		m_Window = Window::Create(windowProps);
		m_Window->SetEventCallback(SP_BIND_CALLBACK(Application::OnEvent));

		Renderer::SetGraphicsAPI(m_Properties.GraphicsAPI);

		Renderer::Init();
		RenderCommand::SetCullMode(m_Properties.CullMode);
		ScriptEngine::Init();
		AudioEngine::Init();

		if (m_Properties.EnableGUI)
		{
			m_GuiLayer = new GuiLayer();
			PushOverlay(m_GuiLayer);
		}
	}

	Application::~Application()
	{
		SP_PROFILE_FUNCTION();

		AudioEngine::Shutdown();
		ScriptEngine::Shutdown();
		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		SP_PROFILE_FUNCTION();

		layer->OnAttach();
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		SP_PROFILE_FUNCTION();

		overlay->OnAttach();
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::Close()
	{
		m_Running = false;
		g_ApplicationRunning = false;
	}

	void Application::OnEvent(Event& e)
	{
		SP_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(SP_BIND_CALLBACK(Application::OnWindowCloseEvent));
		dispatcher.Dispatch<WindowResizeEvent>(SP_BIND_CALLBACK(Application::OnWindowResizeEvent));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (e.Handled)
				break;

			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		SP_PROFILE_FUNCTION();

		while (m_Running)
		{
			SP_PROFILE_SCOPE("Application Loop");

			float time = Time::GetTime();
			TimeStep delta = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_ApplicationMinimized)
			{
				SP_PROFILE_SCOPE("LayerStack OnUpdate");

				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(delta);
			}

			if (m_Properties.EnableGUI)
			{
				m_GuiLayer->BeginFrame();
				{
					SP_PROFILE_SCOPE("LayerStack OnGuiRender");

					for (Layer* layer : m_LayerStack)
						layer->OnGuiRender();
				}
				m_GuiLayer->EndFrame();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		m_Running = false;
		g_ApplicationRunning = false;
		return true;
	}

	bool Application::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		SP_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_ApplicationMinimized = true;
			return false;
		}

		m_ApplicationMinimized = false;

		Viewport viewport;
		viewport.TopLeftXPos = 0;
		viewport.TopLeftYPos = 0;
		viewport.Width = e.GetWidth();
		viewport.Height = e.GetHeight();

		Renderer::OnWindowResize(viewport);

		return false;
	}

}
