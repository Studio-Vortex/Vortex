#include "sppch.h"
#include "Sparky/Core/Window.h"

#ifdef SP_PLATFORM_WINDOWS
	#include "Platform/Windows/WindowsWindow.h"
#endif // SP_PLATFORM_WINDOWS

namespace Sparky
{
	UniqueRef<Window> Window::Create(const WindowProperties& props)
	{
#ifdef SP_PLATFORM_WINDOWS
		return CreateUnique<WindowsWindow>(props);
#else
		SP_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif // SP_PLATFORM_WINDOWS
	}

}
