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
		if (Sparky::Input::IsKeyPressed(SP_KEY_ESCAPE))
			Sparky::Application::Get().CloseApplication();
	}

	void OnEvent(Sparky::Event& event) override
	{
		if (event.GetEventType() == Sparky::EventType::KeyPressed)
		{
			Sparky::KeyPressedEvent& e = (Sparky::KeyPressedEvent&)event;
			SP_TRACE("{}", (char)e.GetKeyCode());
		}
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