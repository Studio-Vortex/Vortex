#pragma once

#include "Application.h"

#ifdef SP_PLATFORM_WINDOWS

	extern Sparky::Application* Sparky::CreateApplication();

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		SP_PROFILE_BEGIN_SESSION("Startup", "SparkyProfile-Startup.json");
		auto app = Sparky::CreateApplication();
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Runtime", "SparkyProfile-Runtime.json");
		app->Run();
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Shutdown", "SparkyProfile-Shutdown.json");
		delete app;
		SP_PROFILE_END_SESSION();
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