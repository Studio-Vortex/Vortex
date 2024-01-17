#pragma once

#include "Base.h"

#include "Vortex/Core/Window.h"
#include "Vortex/Core/LayerStack.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Events/WindowEvent.h"

#include "Vortex/Renderer/RendererAPI.h"

#include "Vortex/Gui/GuiLayer.h"

#include "Vortex/stl/function_queue.h"

int main(int argc, char** argv);

namespace Vortex {

	struct VORTEX_API FrameTime
	{
		TimeStep DeltaTime = 0.0f;
		float ScriptUpdateTime = 0.0f;
		float PhysicsUpdateTime = 0.0f;

		void Clear()
		{
			*this = FrameTime();
		}
	};

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
		std::string WorkingDirectory = "";
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

		VX_FORCE_INLINE GuiLayer* GetGuiLayer() { return m_GuiLayer; }

		VX_FORCE_INLINE static Application& Get() { return *s_Instance; }
		VX_FORCE_INLINE Window& GetWindow() { return *m_Window; }
		VX_FORCE_INLINE const Window& GetWindow() const { return *m_Window; }

		VX_FORCE_INLINE const ApplicationProperties& GetProperties() const { return m_Properties; }

		VX_FORCE_INLINE bool IsRuntime() const { return m_Properties.IsRuntime; }
		VX_FORCE_INLINE bool IsMinimized() const { return m_ApplicationMinimized; }

		void Close();

		VX_FORCE_INLINE vxstl::function_queue<void>& GetPreUpdateFunctionQueue() const { return m_MainThreadPreUpdateFunctionQueue; }
		VX_FORCE_INLINE vxstl::function_queue<void>& GetPostUpdateFunctionQueue() const { return m_MainThreadPostUpdateFunctionQueue; }

		// TODO: think of a better place to put these
		VX_FORCE_INLINE std::string GetEditorBinaryPath() const { return "Vortex-Editor.exe"; }
		VX_FORCE_INLINE std::string GetRuntimeBinaryPath() const { return "Vortex-Runtime.exe"; }

		void AddModule(const SubModule& submodule);
		void RemoveModule(const SubModule& submodule);
		const ModuleLibrary& GetModuleLibrary() const;

		VX_FORCE_INLINE const FrameTime& GetFrameTime() const { return m_FrameTime; }
		VX_FORCE_INLINE FrameTime& GetFrameTime() { return m_FrameTime; }

	private:
		i32 Run();

		bool OnWindowCloseEvent(WindowCloseEvent& e);
		bool OnWindowResizeEvent(WindowResizeEvent& e);

	private:
		inline static Application* s_Instance = nullptr;

	private:
		FrameTime m_FrameTime;
		ApplicationProperties m_Properties;
		UniqueRef<Window> m_Window = nullptr;
		GuiLayer* m_GuiLayer = nullptr;
		LayerStack m_Layers;

		ModuleLibrary m_ModuleLibrary;

		mutable vxstl::function_queue<void> m_MainThreadPreUpdateFunctionQueue;
		mutable vxstl::function_queue<void> m_MainThreadPostUpdateFunctionQueue;

		float m_LastFrameTimeStamp = 0.0f;

		bool m_Running = false;
		bool m_ApplicationMinimized = false;

	private:
		friend int ::main(int argc, char** argv);
	};

	// To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);

}
