#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Sparky {

	class SparkyEditor : public Application
	{
	public:
		SparkyEditor(const ApplicationProperties& props)
			: Application(props)
		{
			PushLayer(new EditorLayer());
		}
	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationProperties props;
		// TODO: Remove this, just for convenience
#ifdef SP_DEBUG
		props.Name = "Sparky Editor - (Debug devel x64)";
#elif SP_RELEASE
		props.Name = "Sparky Editor - (Release x64)";
#elif SP_DIST
		props.Name = "Sparky Editor";
#endif
		props.WindowWidth = 1600;
		props.WindowHeight = 900;
		props.MaximizeWindow = true;
		props.WindowDecorated = true;
		props.VSync = true;
		props.EnableGUI = true;
		props.GraphicsAPI = RendererAPI::API::OpenGL;

		props.WorkingDirectory = "C:/dev/Sparky Engine";
		props.CommandLineArgs = args;

		return new SparkyEditor(props);
	}

}
