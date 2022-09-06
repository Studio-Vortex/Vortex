#pragma once

#include "Core.h"

#include "Sparky/Window.h"
#include "Sparky/LayerStack.h"
#include "Sparky/Events/ApplicationEvent.h"

#include "Sparky/Gui/GuiLayer.h"

#include "Sparky/Renderer/Shader.h"
#include "Sparky/Renderer/Buffer.h"

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

		unsigned int m_VertexArray;
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<Shader> m_Shader;

	private:
		static Application* s_Instance;
	};

	// To be defined in CLIENT
	Application* CreateApplication();

}