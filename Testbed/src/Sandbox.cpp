#include "Sandbox.h"

using namespace Vortex;

Sandbox::Sandbox()
	: Layer("Sandbox")
{
}

void Sandbox::OnAttach()
{
	audioSource = SharedReference<AudioSource>::Create("Projects/Alterverse/Assets/Audio/imarealone.mp3");
}

void Sandbox::OnDetach()
{
}

void Sandbox::OnUpdate(Vortex::TimeStep delta)
{
	RenderCommand::Clear();
}

void Sandbox::OnGuiRender()
{
	ImGuiViewport* viewport = Gui::GetMainViewport();
	Gui::SetNextWindowPos(viewport->WorkPos);
	Gui::SetNextWindowSize(viewport->WorkSize);
	Gui::SetNextWindowViewport(viewport->ID);

	Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

	uint32_t flags = ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoBringToFrontOnFocus;
	Gui::Begin("Audio Test", nullptr, flags);

	Gui::Text(audioSource->GetAudioClip().Name.c_str());
	float len = audioSource->GetAudioClip().Length;
	UI::BeginPropertyGrid();
	UI::Property("Length", len);
	UI::EndPropertyGrid();

	if (Gui::Button("Play")) {
		if (!audioSource->IsPlaying())
		{
			audioSource->Play();
		}
	}

	Gui::End();

	Gui::PopStyleVar(3);
}

void Sandbox::OnEvent(Vortex::Event& e)
{
}
