#pragma once

#include "Base.h"

#include "Sparky/Core/Window.h"
#include "Sparky/Core/LayerStack.h"
#include "Sparky/Events/ApplicationEvent.h"
#include "Sparky/Renderer/RendererAPI.h"

#include "Sparky/Gui/GuiLayer.h"

int main(int argc, char* argv[]);

namespace Sparky {

	struct SPARKY_API ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			SP_CORE_ASSERT(index < Count, "Index out of bounds!");
			return Args[index];
		}
	};

	struct SPARKY_API ApplicationProperties
	{
		std::string Name = "Sparky Application";
		uint16_t WindowWidth = 1600;
		uint16_t WindowHeight = 900;
		uint32_t SampleCount = 4;
		bool MaximizeWindow = false;
		bool WindowDecorated = true;
		bool VSync = true;
		bool EnableGUI = false;
		RendererAPI::API GraphicsAPI = RendererAPI::API::OpenGL;
		std::string WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class SPARKY_API Application
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

		void Quit();

		void SubmitToMainThread(const std::function<void()>& func);

		// TODO: think of a better place to put these
		const std::string& GetEditorAppPath() const { return "Resources/Bin/Sparky-Editor.exe"; }
		const std::string& GetRuntimeAppPath() const { return "Resources/Bin/Sparky-Runtime.exe"; }

	private:
		void Run();
		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();

	private:
		ApplicationProperties m_Properties;
		UniqueRef<Window> m_Window;
		GuiLayer* m_GuiLayer;
		LayerStack m_LayerStack;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		float m_LastFrameTime = 0.0f;
		bool m_Running = true;
		bool m_ApplicationMinimized = false;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT as part of Application Initialization
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
