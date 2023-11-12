#pragma once

#include "Base.h"

#include "Vortex/Core/Window.h"
#include "Vortex/Core/LayerStack.h"

#include "Vortex/Events/WindowEvent.h"

#include "Vortex/Renderer/RendererAPI.h"

#include "Vortex/Gui/GuiLayer.h"

int main(int argc, char* argv[]);

namespace Vortex {

	struct VORTEX_API ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			VX_CORE_ASSERT(index < Count, "Index out of bounds!");
			return Args[index];
		}
	};

	struct VORTEX_API ApplicationProperties
	{
		std::string Name = "Vortex Application";
		uint16_t WindowWidth = 1600;
		uint16_t WindowHeight = 900;
		uint32_t SampleCount = 1;
		bool MaximizeWindow = false;
		bool WindowDecorated = true;
		bool WindowResizable = true;
		bool VSync = true;
		bool EnableGUI = true;
		bool IsRuntime = false;
		RendererAPI::API GraphicsAPI = RendererAPI::API::OpenGL;
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class VORTEX_API Application
	{
	public:
		Application(const ApplicationProperties& props = ApplicationProperties());
		virtual ~Application();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void OnEvent(Event& e);

		GuiLayer* GetGuiLayer() { return m_GuiLayer; }

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		const ApplicationProperties& GetProperties() const { return m_Properties; }

		inline bool IsRuntime() { return m_Properties.IsRuntime; }

		void Quit();

		void SubmitToMainThreadQueue(const std::function<void()>& func);

		// TODO: think of a better place to put these
		std::string GetEditorBinaryPath() const { return "Vortex-Editor.exe"; }
		std::string GetRuntimeBinaryPath() const { return "Vortex-Runtime.exe"; }

	private:
		void ExecuteMainThreadQueue();
		
		void Run();
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		ApplicationProperties m_Properties;
		UniqueRef<Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		float m_LastFrameTime = 0.0f;
		bool m_Running = false;
		bool m_ApplicationMinimized = false;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT as part of Application Initialization
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
