#pragma once

#include "Vortex/Core/Base.h"
#include "Application.h"

#ifdef VX_PLATFORM_WINDOWS

	extern Vortex::Application* Vortex::CreateApplication(Vortex::ApplicationCommandLineArgs);
	bool g_ApplicationRunning = true;

	int main(int argc, char* argv[])
	{
		Vortex::InitalizeEngine();

		while (g_ApplicationRunning)
		{
			SP_PROFILE_BEGIN_SESSION("Startup", "VortexProfile-Startup.json");
			auto app = Vortex::CreateApplication({ argc, argv });
			SP_PROFILE_END_SESSION();

			SP_PROFILE_BEGIN_SESSION("Runtime", "VortexProfile-Runtime.json");
			app->Run();
			SP_PROFILE_END_SESSION();

			SP_PROFILE_BEGIN_SESSION("Shutdown", "VortexProfile-Shutdown.json");
			delete app;
			SP_PROFILE_END_SESSION();
		}

		Vortex::ShutdownEngine();
	}

#elif SP_PLATFORM_MACOS

	extern Vortex::Application* Vortex::CreateApplication();

	int main(int argc, char* argv[])
	{
		Vortex::Log::Init();

		SP_PROFILE_BEGIN_SESSION("Startup", "VortexProfile-Startup.json");
		auto app = Vortex::CreateApplication({ argc, argv });
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Runtime", "VortexProfile-Runtime.json");
		app->Run();
		SP_PROFILE_END_SESSION();

		SP_PROFILE_BEGIN_SESSION("Shutdown", "VortexProfile-Shutdown.json");
		delete app;
		SP_PROFILE_END_SESSION();
	}

#endif // VX_PLATFORM_WINDOWS
