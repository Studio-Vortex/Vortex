#include "vxpch.h"
#include "GuiLayer.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Gui/Colors.h"
#include "Vortex/Editor/UI/UI.h"

#include "Vortex/Editor/FontAwesome.h"

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
		
		struct UIFont
		{
			const char* Filepath = "";
			float FontSize = 16.0f;
			bool IsDefault = false;
		};

		std::vector<UIFont> fonts =
		{
			{ "Resources/Fonts/opensans/OpenSans-Bold.ttf", 18.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Regular.ttf", 22.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Regular.ttf", 26.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Italic.ttf", 18.0f, false },
			{ "Resources/Fonts/roboto/Roboto-Regular.ttf", 18.0f, false },
			{ "Resources/Fonts/Arial.ttf", 17.0f, false },
			{ "Resources/Fonts/Arial.ttf", 18.0f, false },
			{ "Resources/Fonts/Arial.ttf", 19.0f, false },
			{ "Resources/Fonts/Arial.ttf", 20.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Medium.ttf", 17.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Medium.ttf", 18.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Medium.ttf", 19.0f, false },
			{ "Resources/Fonts/opensans/OpenSans-Medium.ttf", 20.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 17.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 18.0f, true },
			{ "Resources/Fonts/SegoeUI.ttf", 19.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 20.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 21.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 22.0f, false },
			{ "Resources/Fonts/SegoeUI.ttf", 24.0f, false },
		};

		for (const UIFont& font : fonts)
		{
			ImFont* f = io.Fonts->AddFontFromFileTTF(font.Filepath, font.FontSize);
			
			MergeIconFontWithLast();

			if (!font.IsDefault)
				continue;
			
			io.FontDefault = f;
		}

		ImGuiStyle& style = ImGui::GetStyle();

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			style.FrameRounding = 5.0f;
		}

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		style.WindowTitleAlign = { 0.5f, 0.5f };
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.GrabRounding = 2.5f;

		ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_Float);

		SetDarkThemeColors();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindowHandle());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 460");

		UI::Internal::Init();
	}

	void GuiLayer::OnDetach()
	{
		VX_PROFILE_FUNCTION();

		UI::Internal::Shutdown();

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
		auto& style = ImGui::GetStyle();
		auto& colors = ImGui::GetStyle().Colors;

		//========================================================
		/// Colours

		// Headers
		colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);

		// Buttons
		colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
		colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
		colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
		colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);

		// Tabs
		colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
		colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
		colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

		// Title
		colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

		// Checkbox
		colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

		// Separator
		colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);
		colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::highlight);
		colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Window Background
		colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::background);
		colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundPopup);
		colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

		// Tables
		colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

		// Menubar
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		//========================================================
		/// Style
		style.FrameRounding = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing = 11.0f;
	}

	void GuiLayer::SetLightGrayThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.95f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
		colors[ImGuiCol_Button] = ImVec4(0.2f, 0.4f, 0.75f, 0.65f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(.15, .15, .15, 1);
		colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.4f, 0.75f, 0.5f);
		colors[ImGuiCol_TabActive] = ImVec4(0.2f, 0.4f, 0.75f, 1.0f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.4f, 0.75f, 0.65f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.5f, 0.5f, 0.5f, 0.25f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	}

	void GuiLayer::MergeIconFontWithLast()
	{
		ImGuiIO& io = ImGui::GetIO();

		ImFontConfig config;
		config.MergeMode = true;
		const ImWchar iconRanges[] = { VX_ICON_MIN, VX_ICON_MAX, 0 };
		io.Fonts->AddFontFromFileTTF(VX_FONT_ICON_FILE_NAME, 16.0f, &config, iconRanges);
		io.Fonts->Build();
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
