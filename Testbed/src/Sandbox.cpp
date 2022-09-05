#include <Sparky.h>

class ExampleLayer : public Sparky::Layer
{
public:
	ExampleLayer()
		: Layer("Example") {}

	~ExampleLayer() override
	{
	
	}

	void OnUpdate() override
	{
		SP_INFO("ExampleLayer::Update");
	}

	void OnEvent(Sparky::Event& event) override
	{
		SP_TRACE(event);
	}
};

class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Sparky::ImGuiLayer());
	}

	~Sandbox() override
	{

	}
};

Sparky::Application* Sparky::CreateApplication() {
	return new Sandbox();
}