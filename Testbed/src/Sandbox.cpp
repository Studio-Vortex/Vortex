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

	void OnGuiRender() override
	{
		Gui::Begin("Hello");
		Gui::Text("Welcome to Sparky!");
		Gui::End();
	}

	void OnEvent(Sparky::Event& event) override
	{
		if (event.GetEventType() == Sparky::EventType::KeyPressed)
		{
			Sparky::KeyPressedEvent& e = (Sparky::KeyPressedEvent&)event;
			SP_TRACE("{}", (char)e.GetKeyCode());
			
			vec2 a(10, 10);
			vec2 b(10, 10);
			SP_TRACE("Hello {}", a);
		}
	}
};

class Sandbox : public Sparky::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox() override
	{

	}
};

Sparky::Application* Sparky::CreateApplication() {
	return new Sandbox();
}