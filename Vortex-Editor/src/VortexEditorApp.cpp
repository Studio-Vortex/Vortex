#include <Vortex.h>
#include <Vortex/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Vortex {

	class VortexEditor : public Application
	{
	public:
		VortexEditor(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationProperties props;

		// Just for convenience
#ifdef VX_DEBUG
		props.Name = fmt::format("Vortex Editor - (Debug devel - {})", VORTEX_BUILD_ID);
#elif VX_RELEASE
		props.Name = fmt::format("Vortex Editor - (Release - {0})", VORTEX_BUILD_ID);
#elif VX_DIST
		props.Name = fmt::format("Vortex Editor ({0})", VORTEX_BUILD_ID);
#endif

		props.WindowWidth = 1600;
		props.WindowHeight = 900;
		props.SampleCount = 1;
		props.MaximizeWindow = true;
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

		return new VortexEditor(props);
	}

}
