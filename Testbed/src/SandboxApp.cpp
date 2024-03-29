#include <Vortex.h>
#include <Vortex/Core/EntryPoint.h>

#include "Sandbox.h"

class SandboxApp : public Vortex::Application
{
public:
	SandboxApp(const Vortex::ApplicationProperties& properties)
		: Application(properties)
	{
		PushLayer(new Sandbox());
	}
};

Vortex::Application* Vortex::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationProperties props;
	props.Name = "Sandbox";
	props.WindowWidth = 1600;
	props.WindowHeight = 900;
	props.MaximizeWindow = false;
	props.WindowDecorated = true;
	props.VSync = true;
	props.EnableGUI = true;
	props.IsRuntime = false;
	props.GraphicsAPI = RendererAPI::API::OpenGL;

	props.WorkingDirectory = "C:/dev/Vortex Engine";
	props.CommandLineArgs = args;

	return new SandboxApp(props);
}
