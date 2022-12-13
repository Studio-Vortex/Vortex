#pragma once

#include "vxpch.h"

#include "Vortex/Core/Base.h"

#include "Vortex/Events/Event.h"
#include "Vortex/Core/Math.h"

namespace Vortex {

	struct WindowProperties
	{
		std::string Title;
		Math::vec2 Size;
		Math::vec2 Position;
		bool Maximized;
		bool VSync;
		bool Decorated;
	};

	// Interface representing a desktop system based window
	class VORTEX_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual const Math::vec2& GetSize() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual const Math::vec2& GetPosition() const = 0;

		virtual bool IsMaximized() const = 0;
		virtual void SetMaximized(bool maximized) = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetTitle(const std::string& title) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void CenterWindow() const = 0;

		virtual void* GetNativeWindowHandle() const = 0;

		virtual void SetCursorPosition(uint32_t mouseCursorX, uint32_t mouseCursorY) const = 0;

		virtual void ShowMouseCursor(bool enabled, bool rawInputMode = false) const = 0;

		static UniqueRef<Window> Create(const WindowProperties& props = WindowProperties());
	};

}
