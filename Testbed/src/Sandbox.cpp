#include "Sandbox.h"

using namespace Vortex;

Sandbox::Sandbox()
	: Layer("Sandbox")
{
}

void Sandbox::OnAttach()
{
	audioSource = AudioSource::Create("Projects/Alterverse/Assets/Audio/darkdayroad.mp3");
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

	Wave::PlaybackDevice device = audioSource->GetPlaybackDevice();
	std::string path = audioSource->GetPath().string();
	Gui::Text(path.c_str());
	// TODO we also need to make Sound::GetLength() faster
	//float len = device.GetSound().GetLength();
	//UI::BeginPropertyGrid();
	//UI::Property("Length", len);
	//UI::EndPropertyGrid();

	if (Gui::Button("Play")) {
		device.Play();
	}

	Gui::End();

	Gui::PopStyleVar(3);
}

bool Sandbox::OnWindowClose(WindowCloseEvent& e)
{
	audioSource->GetPlaybackDevice().Shutdown(Audio::GetContext());
	return false;
}

void Sandbox::OnEvent(Vortex::Event& e)
{
	EventDispatcher dispatcher(e);

	dispatcher.Dispatch<WindowCloseEvent>(VX_BIND_CALLBACK(Sandbox::OnWindowClose));
}
