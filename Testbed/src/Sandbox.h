#pragma once

#include <Vortex.h>

class Sandbox : public Vortex::Layer
{
public:
	Sandbox();
	~Sandbox() override = default;

	void OnAttach() override;
	void OnDetach() override;

	void OnUpdate(Vortex::TimeStep ts) override;
	void OnGuiRender() override;
	void OnEvent(Vortex::Event &e) override;
	Vortex::SharedReference<Vortex::AudioSource> audioSource;
};
