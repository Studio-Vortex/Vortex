#pragma once

#include "Core.h"

#include "Sparky/Window.h"
#include "Sparky/LayerStack.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/Gui/GuiLayer.h"

#include "Sparky/Renderer/VertexArray.h"
#include "Sparky/Renderer/Buffer.h"
#include "Sparky/Renderer/Shader.h"

namespace Sparky {

	class SPARKY_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		inline void CloseApplication() { m_Running = false; }

	private:
		bool OnWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;
		bool m_Running = true;

		std::shared_ptr<VertexArray> m_TriangleVA;
		std::shared_ptr<Shader> m_Shader;

		std::shared_ptr<VertexArray> m_SquareVA;
		std::shared_ptr<Shader> m_Shader2;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}