#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "LauncherLayer.h"

namespace Sparky {

	class SparkyLauncher : public Application
	{
	public:
		SparkyLauncher(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new LauncherLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationProperties props;
		// TODO: Remove this, just for convenience
#ifdef SP_DEBUG
		props.Name = "Sparky Launcher - (Debug devel x64)";
#elif SP_RELEASE
		props.Name = "Sparky Launcher - (Release x64)";
#elif SP_DIST
		props.Name = "Sparky Launcher";
#endif
		props.WindowWidth = 1000;
		props.WindowHeight = 600;
		props.MaximizeWindow = false;
		props.WindowDecorated = true;
		props.VSync = true;
		props.EnableGUI = true;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		props.CommandLineArgs = args;

		return new SparkyLauncher(props);
	}

}
