#include <Sparky.h>
#include <Sparky/Core/EntryPoint.h>

#include "ExampleLayer.h"
#include "Sandbox2D.h"

class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		//PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2D());
	}

	~Sandbox() override
	{

	}
};

Sparky::Application* Sparky::CreateApplication() {
	return new Sandbox();
}