#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "ExampleLayer.h"
#include "Sandbox2D.h"

class Sandbox : public Sparky::Application
{
public:
	Sandbox(const Sparky::ApplicationProperties& properties)
		: Application(properties)
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}
};

Sparky::Application* Sparky::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationProperties props;
	props.Name = "Sandbox";
	props.MaximizeWindow = true;
	props.WindowDecorated = false;
	props.VSync = true;
	props.EnableGUI = true;

	props.WorkingDirectory = "../Sparky-Editor";
	props.CommandLineArgs = args;

	return new Sandbox(props);
}