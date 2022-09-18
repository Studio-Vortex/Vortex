#pragma once

#include "Sparky/Core/Base.h"
#include "Application.h"

#ifdef SP_PLATFORM_WINDOWS

	extern Sparky::Application* Sparky::CreateApplication(Sparky::ApplicationCommandLineArgs);

	int main(int argc, char* argv[])
	{
		Sparky::Log::Init();

		SP_PROFILE_BEGIN_SESSION("Startup", "SparkyProfile-Startup.json");
		auto app = Sparky::CreateApplication({ argc, argv });
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

		SP_PROFILE_BEGIN_SESSION("Startup", "SparkyProfile-Startup.json");
		auto app = Sparky::CreateApplication({ argc, argv });
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Runtime", "SparkyProfile-Runtime.json");
		app->Run();
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Shutdown", "SparkyProfile-Shutdown.json");
		delete app;
		SP_PROFILE_END_SESSION();
	}

#endif // SP_PLATFORM_WINDOWS
