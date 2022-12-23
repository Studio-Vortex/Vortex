#include "vxpch.h"
#include "GuiLayer.h"

#include "Vortex/Core/Application.h"

#include <imgui.h>

#define IMGUI_IMPL_API
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

// Temporary
#include <Glad/glad.h>
#include <GLFW/glfw3.h>

namespace Vortex {

	GuiLayer::GuiLayer()
		: Layer("GuiLayer") {}

	GuiLayer::~GuiLayer() {}

	void GuiLayer::OnAttach()
	{
		VX_PROFILE_FUNCTION();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Regular.ttf", 22.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Regular.ttf", 26.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Italic.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/roboto/Roboto-Regular.ttf", 18.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/Arial.ttf", 17.0f);
		io.Fonts->AddFontFromFileTTF("Resources/Fonts/SegoeUI.ttf", 18.0f);
		const auto& regular = io.Fonts->AddFontFromFileTTF("Resources/Fonts/opensans/OpenSans-Regular.ttf", 18.0f);
		io.FontDefault = regular;

		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		style.WindowTitleAlign = { 0.5f, 0.5f };
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.GrabRounding = 2.5f;

		ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_Float);

		SetDarkThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindowHandle());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");
	}

	void GuiLayer::OnDetach()
	{
		VX_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void GuiLayer::OnEvent(Event& event)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			event.Handled |= event.IsInCategory(EventCategoryMouse) & io.WantCaptureMouse;
			event.Handled |= event.IsInCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
		}
	}

	void GuiLayer::BeginFrame()
	{
		VX_PROFILE_FUNCTION();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();
	}

	void GuiLayer::EndFrame()
	{
		VX_PROFILE_FUNCTION();

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		auto size = app.GetWindow().GetSize();
		io.DisplaySize = ImVec2(size.x, size.y);

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupContext);
		}
	}

	uint32_t GuiLayer::GetActiveLayerID() const
	{
		ImGuiContext& g = *GImGui;
		return g.ActiveId;
	}

	void GuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4{ 1, 1, 1, 1 };
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };
		colors[ImGuiCol_PopupBg] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.14f, 0.14f, 0.14f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		
		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	void GuiLayer::SetLightGrayThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1, 1, 1, 1);
		colors[ImGuiCol_WindowBg] = ImVec4(.2, .2, .2, .95);
		colors[ImGuiCol_PopupBg] = ImVec4(.15, .15, .15, 1);
		colors[ImGuiCol_Button] = ImVec4(.2, .4, .75, .65);
		colors[ImGuiCol_TitleBg] = ImVec4(.1, .1, .1, 1);
		colors[ImGuiCol_TitleBgActive] = ImVec4(.15, .15, .15, 1);
		colors[ImGuiCol_Tab] = ImVec4(.2, .4, .75, .5);
		colors[ImGuiCol_TabActive] = ImVec4(.2, .4, .75, 1);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(.2, .4, .75, .65);
		colors[ImGuiCol_FrameBg] = ImVec4(.5, .5, .5, .25);
		colors[ImGuiCol_MenuBarBg] = ImVec4(.15, .15, .15, 1);
	}

}

namespace ImGui {

	void TextCentered(const char* text, float y, ...)
	{
		va_list args;
		va_start(args, text);
		ImVec2 textSize = CalcTextSize(text);
		SetCursorPos({ (GetWindowWidth() / 2.0f) - (textSize.x / 2.0f), y });
		TextV(text, args);
		va_end(args);
	}

}
