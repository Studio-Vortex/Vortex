#include <Vortex.h>
#include <Vortex/Core/EntryPoint.h>

#include "LauncherLayer.h"

namespace Vortex {

	class VortexLauncher : public Application
	{
	public:
		VortexLauncher(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new LauncherLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationProperties props;
		// TODO: Remove this, just for convenience
#ifdef VX_DEBUG
		props.Name = "Vortex Launcher - (Debug devel x64)";
#elif VX_RELEASE
		props.Name = "Vortex Launcher - (Release x64)";
#elif VX_DIST
		props.Name = "Vortex Launcher";
#endif
		props.WindowWidth = 1000;
		props.WindowHeight = 600;
		props.MaximizeWindow = false;
		props.WindowDecorated = true;
		props.VSync = true;
		props.EnableGUI = true;
		props.IsRuntime = false;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		{
			// Note this is here for debugging purposes inside visual studio otherwise it can be removed
			props.WorkingDirectory = "C:/dev/Vortex Engine";
		}
		props.CommandLineArgs = args;

		return new VortexLauncher(props);
	}

}
