#include "SettingsPanel.h"

namespace Sparky {

	void SettingsPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Settings", &s_ShowPanel);

			Gui::PushFont(boldFont);
			Gui::Text("Renderer 2D");
			Gui::PopFont();

			if (Gui::ColorEdit3("Clear Color", Math::ValuePtr(m_Settings.ClearColor)))
				RenderCommand::SetClearColor(m_Settings.ClearColor);

			float lineWidth = Renderer2D::GetLineWidth();
			if (Gui::DragFloat("Line width", &lineWidth, 0.1f, 0.1f, 4.0f, "%.2f"))
				Renderer2D::SetLineWidth(lineWidth);

			static bool wireframeMode = false;
			if (Gui::Checkbox("Show wireframe", &wireframeMode))
				RenderCommand::SetWireframe(wireframeMode);

			static bool vsync = true;
			if (Gui::Checkbox("Use VSync", &vsync))
				Application::Get().GetWindow().SetVSync(vsync);
			
			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Physics 2D");
			Gui::PopFont();

			Gui::ColorEdit4("Collider color", Math::ValuePtr(m_Settings.ColliderColor));

			static Math::vec2 gravity = Scene::GetPhysicsWorldGravity();
			if (Gui::DragFloat2("Gravity", Math::ValuePtr(gravity), 0.1f))
				Scene::SetPhysicsWorldGravitty(gravity);

			static int32_t velocityIterations = Scene::GetPhysicsWorldVeloctiyIterations();
			if (Gui::DragInt("Velocity Iterations", &velocityIterations, 1.0f, 1, 100))
				Scene::SetPhysicsWorldVelocityIterations(velocityIterations);

			static int32_t positionIterations = Scene::GetPhysicsWorldPositionIterations();
			Gui::DragInt("Position Iterations", &positionIterations, 1.0f, 1, 100);
				Scene::SetPhysicsWorldPositionIterations(positionIterations);

			Gui::Checkbox("Show physics colliders", &m_Settings.ShowColliders);

			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Editor");
			Gui::PopFont();

			Gui::DragFloat("Editor FOV", &m_Settings.EditorCameraFOV, 0.25f, 4.0f, 120.0f, "%.2f");

			Gui::Spacing();

			if (Gui::BeginMenu("Theme"))
			{
				if (Gui::MenuItem("Dark"))
					Application::Get().GetGuiLayer()->SetDarkThemeColors();
				if (Gui::MenuItem("Light Gray"))
					Application::Get().GetGuiLayer()->SetLightGrayThemeColors();
				if (Gui::MenuItem("Default"))
					Gui::StyleColorsDark();
				if (Gui::MenuItem("Classic"))
					Gui::StyleColorsClassic();
				if (Gui::MenuItem("Light"))
					Gui::StyleColorsLight();

				Gui::EndMenu();
			}

			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Build");
			Gui::PopFont();

			char buffer[256];
			strcpy(buffer, m_Settings.CurrentEditorScenePath.string().c_str());
			Gui::InputText("Active Scene", buffer, 256, ImGuiInputTextFlags_ReadOnly);

			if (Gui::Button("Launch Runtime"))
				m_Settings.LaunchRuntimeCallback();

			Gui::End();
		}
	}

}
