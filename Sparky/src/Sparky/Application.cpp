#include "sppch.h"
#include "Application.h"

#include "Sparky/Input.h"
#include "Sparky/Renderer/Renderer.h"
#include "Sparky/KeyCodes.h"
#include "Sparky/Core/TimeStep.h"

#include <Glad/glad.h>
#include <GLFW/glfw3.h>

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
		dispatcher.Dispatch<WindowCloseEvent>(SP_BIND_CALLBACK(Application::OnWindowCloseEvent));

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
			float time = (float)glfwGetTime();
			TimeStep timeStep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timeStep);

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

	bool Application::OnWindowCloseEvent(WindowCloseEvent& e)
	{
		CloseApplication();
		return true;
	}

}