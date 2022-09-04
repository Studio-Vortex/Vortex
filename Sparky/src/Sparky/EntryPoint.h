#pragma once

#include "Application.h"

#ifdef SP_PLATFORM_WINDOWS

extern Sparky::Application* Sparky::CreateApplication();

int main(int argc, char* argv[])
{
	auto app = Sparky::CreateApplication();

	app->Run();

	delete app;
}

#endif // SP_PLATFORM_WINDOWS
