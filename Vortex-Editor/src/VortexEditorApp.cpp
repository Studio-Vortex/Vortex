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
		// TODO: Remove this, just for convenience
#ifdef VX_DEBUG
		props.Name = "Vortex Editor - (Debug devel x64)";
#elif VX_RELEASE
		props.Name = "Vortex Editor - (Release x64)";
#elif VX_DIST
		props.Name = "Vortex Editor";
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

		props.WorkingDirectory = "C:/dev/Vortex Engine";
		props.CommandLineArgs = args;

		return new VortexEditor(props);
	}

}
