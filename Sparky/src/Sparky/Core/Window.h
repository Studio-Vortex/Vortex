#pragma once

#include "sppch.h"

#include "Sparky/Core/Base.h"

#include "Sparky/Events/Event.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	struct WindowProps
	{
		std::string Title;
		Math::vec2 Size;

		WindowProps(
			const std::string& title = "Sparky Engine",
			const Math::vec2& size = Math::vec2(1600.0f, 900.0f)
		) : Title(title), Size(size) {}
	};

	// Interface representing a desktop system based window
	class SPARKY_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;

		virtual Math::vec2 GetSize() const = 0;
		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window Attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetNativeWindowHandle() const = 0;

		static UniqueRef<Window> Create(const WindowProps& props = WindowProps());
	};

}