#include "sppch.h"
#include "Application.h"

#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"

#include <GLFW/glfw3.h>

namespace Sparky {

#define BIND_CALLBACK(ident) std::bind(&Application::ident, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = Window::Create();
		m_Window->SetEventCallback(BIND_CALLBACK(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_CALLBACK(OnWindowClose));

		SP_CORE_TRACE(e);
	}

	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(.2, .2, .2, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

}