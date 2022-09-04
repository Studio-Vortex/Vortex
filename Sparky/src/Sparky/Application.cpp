#include "sppch.h"
#include "Application.h"

#include "Sparky/Events/Event.h"
#include "Sparky/Events/MouseEvent.h"
#include "Sparky/Log.h"

namespace Sparky {
	Application::Application()
	{
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		MouseMovedEvent e(1280, 1080);
		SP_TRACE(e);

		while (true);
	}
}