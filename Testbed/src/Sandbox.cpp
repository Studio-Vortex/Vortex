#include <Sparky.h>

class Sandbox : public Sparky::Application
{
public:
	Sandbox() {}
	~Sandbox() override {}
};

Sparky::Application* Sparky::CreateApplication() {
	return new Sandbox();
}