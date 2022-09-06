#include "sppch.h"
#include "Application.h"

#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"

#include "Sparky/Input.h"

#include <Glad/glad.h>

namespace Sparky {

	Application* Application::s_Instance = nullptr;

	Application::Application()
		: m_Window(Window::Create()), m_GuiLayer(new GuiLayer())
	{
		SP_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;

		m_Window->SetEventCallback(SP_BIND_CALLBACK(Application::OnEvent));

		PushOverlay(m_GuiLayer);
	}

	Application::~Application()
	{
		
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(SP_BIND_CALLBACK(Application::OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);

			if (e.Handled)
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(.2f, .2f, .2f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate();

			m_GuiLayer->BeginFrame();
			for (Layer* layer : m_LayerStack)
				layer->OnGuiRender();
			m_GuiLayer->EndFrame();
			
			m_Window->OnUpdate();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}