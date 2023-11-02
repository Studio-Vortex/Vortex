#include "Sandbox.h"

using namespace Vortex;

Sandbox::Sandbox()
	: Layer("Sandbox")
{
}

void Sandbox::OnAttach()
{
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
	Gui::Begin("Network", nullptr, flags);
	
	static bool firstFrame = true;
	if (firstFrame)
	{
		Gui::OpenPopup("Sign In");
		firstFrame = false;
	}

	static std::string name = "";

	if (UI::ShowMessageBox("Sign In", { 500, 300 }))
	{
		UI::BeginPropertyGrid();

		UI::Property("Name", name);

		UI::EndPropertyGrid();

		if (Gui::Button("Sign In"))
		{

		}

		Gui::SameLine();

		if (Gui::Button("Cancel"))
		{
			Gui::CloseCurrentPopup();
		}

		Gui::EndPopup();
	}

	Gui::End();

	Gui::PopStyleVar(3);
}

void Sandbox::OnEvent(Vortex::Event& e)
{
}
