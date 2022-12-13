#include <Vortex.h>
#include <Vortex/Core/EntryPoint.h>

#include "ExampleLayer.h"
#include "Sandbox2D.h"

class Sandbox : public Vortex::Application
{
public:
	Sandbox(const Vortex::ApplicationProperties& properties)
		: Application(properties)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
};

Vortex::Application* Vortex::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationProperties props;
	props.Name = "Sandbox";
	props.MaximizeWindow = true;
	props.WindowDecorated = false;
	props.VSync = true;
	props.EnableGUI = true;

	props.WorkingDirectory = "../Vortex-Editor";
	props.CommandLineArgs = args;

	return new Sandbox(props);
}