#pragma once

#include "Core.h"

#include "Sparky/Core/Window.h"
#include "Sparky/Core/LayerStack.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/Gui/GuiLayer.h"

namespace Sparky {

	class SPARKY_API Application
	{
	public:
		Application();
		virtual ~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void OnEvent(Event& e);

		void Run();

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		void Close();

	private:
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		UniqueRef<Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
		bool m_Running = true;
		bool m_Minimized = false;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}