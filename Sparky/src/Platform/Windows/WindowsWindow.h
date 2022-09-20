#pragma once

#include "Sparky/Core/Window.h"

#include "Sparky/Renderer/GraphicsContext.h"

#include <GLFW/glfw3.h>

namespace Sparky {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline Math::vec2 GetSize() const override { return { (float)GetWidth(), (float)GetHeight() }; }
		inline unsigned int GetWidth() const override { return (unsigned int)m_Data.Size.x; }
		inline unsigned int GetHeight() const override { return (unsigned int)m_Data.Size.y; }
		void SetMaximized(bool maximized) const override;

		// Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline void* GetNativeWindowHandle() const override { return m_Window; }

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
			bool StartMaximized;
			bool VSync;
			bool Decorated;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}
