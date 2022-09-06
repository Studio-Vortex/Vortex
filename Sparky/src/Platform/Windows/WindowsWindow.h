#pragma once

#include "Sparky/Window.h"

#include "Sparky/Renderer/RendererContext.h"

#include <GLFW/glfw3.h>

namespace Sparky {

	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline Math::vec2 GetSize() const override { return { (float)GetWidth(), (float)GetHeight() }; }
		inline unsigned int GetWidth() const override { return (unsigned int)m_Data.Size.x; }
		inline unsigned int GetHeight() const override { return (unsigned int)m_Data.Size.y; }

		// Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline void* GetNativeWindowHandle() const override { return m_Window; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();

	private:
		GLFWwindow* m_Window;
		RendererContext* m_Context;

		struct WindowData
		{
			std::string Title;
			Math::vec2 Size;
			bool VSync;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}
