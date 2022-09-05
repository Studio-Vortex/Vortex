#pragma once

#include "Application.h"

#if defined(SP_PLATFORM_WINDOWS)

	extern Sparky::Application* Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		auto app = Sparky::CreateApplication();
		app->Run();
		delete app;
	}

#elif defined(GLFW_PLATFORM_MACOS)

	extern Sparky::Application* Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		auto app = Sparky::CreateApplication();
		app->Run();
		delete app;
	}

#endif // GLFW_PLATFORM_WINDOWS
