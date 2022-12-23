#include "vxpch.h"
#include "Vortex/Core/Window.h"

#ifdef VX_PLATFORM_WINDOWS
	#include "Vortex/Platform/Windows/WindowsWindow.h"
#endif // VX_PLATFORM_WINDOWS

namespace Vortex
{
	UniqueRef<Window> Window::Create(const WindowProperties& props)
	{
#ifdef VX_PLATFORM_WINDOWS
		return CreateUnique<WindowsWindow>(props);
#else
		VX_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif // VX_PLATFORM_WINDOWS
	}

}
