#pragma once

#include "Base.h"

#include "Sparky/Core/Window.h"
#include "Sparky/Core/LayerStack.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/Gui/GuiLayer.h"

int main(int argc, char* argv[]);

namespace Sparky {

	class SPARKY_API Application
	{
	public:
		Application(const std::string& name = "Sparky App");
		virtual ~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void OnEvent(Event& e);

		GuiLayer* GetGuiLayer() { return m_GuiLayer; }

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		void Close();

	private:
		void Run();
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
		friend int ::main(int argc, char* argv[]);
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}