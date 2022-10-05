#pragma once

#include "Sparky/Core/Window.h"

#include "Sparky/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

#ifdef SP_PLATFORM_WINDOWS
	#define GLFW_EXPOSE_NATIVE_WIN32
	#include <GLFW/glfw3native.h>
#endif

namespace Sparky {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline Math::vec2 GetSize() const override { return { (float)GetWidth(), (float)GetHeight() }; }
		inline unsigned int GetWidth() const override { return (unsigned int)m_Properties.Size.x; }
		inline unsigned int GetHeight() const override { return (unsigned int)m_Properties.Size.y; }
		bool IsMaximized() const override { return m_Properties.Maximized; }
		void SetMaximized(bool maximized) override;

		// Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Properties.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline void* GetNativeWindowHandle() const override { return m_Window; }

		void SetCursorPosition(uint32_t mouseCursorX, uint32_t mouseCursorY) const override;

	private:
		virtual void Init(const WindowProperties& props);
		virtual void Shutdown();

		void LoadWindowIcon();

	private:
		GLFWwindow* m_Window;
		UniqueRef<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string Title;
			Math::vec2 Size;
			bool Maximized;
			bool VSync;
			bool Decorated;

			EventCallbackFn EventCallback;
		};

		WindowData m_Properties;
	};

}
