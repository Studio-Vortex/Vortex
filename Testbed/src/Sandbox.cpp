#include "Sandbox.h"

using namespace Vortex;

Sandbox::Sandbox()
	: Layer("Sandbox")
{
}

void Sandbox::OnAttach()
{
	audioSource = AudioSource::Create();
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

	UI::BeginPropertyGrid();
	static std::string filepath = "Projects/Alterverse/Assets/Audio/darkdayroad.mp3";
	UI::Property("Filepath", filepath);
	UI::EndPropertyGrid();

	if (!filepath.empty() && Gui::Button("Done"))
	{
		audioSource->SetPath(std::filesystem::path(filepath));
	}
	Gui::SameLine();
	if (!audioSource->GetPath().empty() && Gui::Button("Play##m1")) {
		Wave::PlaybackDevice device = audioSource->GetPlaybackDevice();
		device.GetSound().SetLooping(true);
		device.Play();
	}

	Gui::End();

	Gui::PopStyleVar(3);
}

void Sandbox::OnEvent(Vortex::Event& e)
{
}
