#pragma once

#include "Application.h"

#if SP_PLATFORM_WINDOWS

	extern Sparky::Application* Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		auto app = Sparky::CreateApplication();
		app->Run();
		delete app;
	}

#elif SP_PLATFORM_MACOS

	extern Sparky::Application* Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		auto app = Sparky::CreateApplication();
		app->Run();
		delete app;
	}

#endif // SP_PLATFORM_WINDOWS
