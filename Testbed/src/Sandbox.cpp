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

	~Sandbox() override
	{

	}
};

Sparky::Application* Sparky::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationProperties props;
	props.Name = "Sandbox";
	props.WorkingDirectory = "../Sparky-Editor";
	props.CommandLineArgs = args;

	return new Sandbox(props);
}