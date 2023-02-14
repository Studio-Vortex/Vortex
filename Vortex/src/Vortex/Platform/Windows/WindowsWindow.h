#pragma once

#include "Vortex/Core/Window.h"

#include "Vortex/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

#ifdef VX_PLATFORM_WINDOWS
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include <GLFW/glfw3native.h>
#endif

namespace Vortex {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow() override;

		void OnUpdate() override;

		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Properties.EventCallback = callback; }

		// Window Attributes
		inline const std::string& GetTitle() const override { return m_Properties.Title; }
		void SetTitle(const std::string& title) override;

		inline const Math::vec2& GetSize() const override { return m_Properties.Size; }
		inline uint32_t GetWidth() const override { return (uint32_t)m_Properties.Size.x; }
		inline uint32_t GetHeight() const override { return (uint32_t)m_Properties.Size.y; }
		void SetSize(const Math::vec2& size) override;

		inline const Math::vec2& GetPosition() const override { return m_Properties.Position; }

		inline bool IsMaximized() const override { return m_Properties.Maximized; }
		void SetMaximized(bool maximized) override;
		inline bool IsDecorated() const override { return m_Properties.Decorated; }
		void SetDecorated(bool decorated) override;
		inline bool IsResizeable() const override { return m_Properties.Resizeable; }
		void SetResizeable(bool resizeable) override;
		inline bool IsVSyncEnabled() const override { return m_Properties.VSync; }
		void SetVSync(bool enabled) override;

		void CenterWindow() const override;

		inline void* GetNativeWindowHandle() const override { return m_Window; }

	private:
		virtual void Init(const WindowProperties& props);
		virtual void Shutdown();

		void LoadWindowIcon();

	private:
		GLFWwindow* m_Window = nullptr;
		UniqueRef<GraphicsContext> m_Context = nullptr;

		struct WindowData
		{
			std::string Title = "";
			Math::vec2 Size = Math::vec2(0.0f);
			Math::vec2 Position = Math::vec2(0.0f);
			bool Maximized = false;
			bool Decorated = false;
			bool Resizeable = false;
			bool VSync = false;

			EventCallbackFn EventCallback = nullptr;
		};

		WindowData m_Properties;
	};

}
