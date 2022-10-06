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

			static const char* settingChoices[] = {
				"Renderer 2D",
				"Physics 2D",
				"Editor",
				"Runtime",
				"Build",
			};

			// Left
			static uint32_t selectedSetting = 0;
			Gui::BeginChild("Left Pane", ImVec2(150, 0), true);
			uint32_t i = 0;
			for (auto& choice : settingChoices)
			{
				if (Gui::Selectable(choice, selectedSetting == i))
					selectedSetting = i;

				i++;
			}
			Gui::EndChild();

			Gui::SameLine();

			// Right
			Gui::BeginGroup();
			Gui::BeginChild("Right Pane", ImVec2(0, Gui::GetContentRegionAvail().y));
			if (Gui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
			{
				if (Gui::BeginTabItem(settingChoices[selectedSetting]))
				{
					RenderSettingOptions(selectedSetting);
					Gui::EndTabItem();
				}
				Gui::EndTabBar();
			}
			Gui::EndChild();
			Gui::EndGroup();

			Gui::End();
		}
	}

	void SettingsPanel::RenderSettingOptions(uint32_t selectedSetting)
	{
		if (selectedSetting == 0) // Renderer 2D Settings
		{
			if (Gui::ColorEdit3("Clear Color", Math::ValuePtr(m_Settings.ClearColor)))
				RenderCommand::SetClearColor(m_Settings.ClearColor);

			float lineWidth = Renderer2D::GetLineWidth();
			if (Gui::DragFloat("Line Width", &lineWidth, 0.1f, 0.1f, 4.0f, "%.2f"))
				Renderer2D::SetLineWidth(lineWidth);

			static bool wireframeMode = false;
			if (Gui::Checkbox("Show Wireframe", &wireframeMode))
				RenderCommand::SetWireframe(wireframeMode);

			static bool vsync = true;
			if (Gui::Checkbox("Use VSync", &vsync))
				Application::Get().GetWindow().SetVSync(vsync);
		}

		if (selectedSetting == 1) // Physics 2D Settings
		{
			Gui::ColorEdit4("Collider Color", Math::ValuePtr(m_Settings.ColliderColor));

			static Math::vec2 gravity = Scene::GetPhysicsWorldGravity();
			if (Gui::DragFloat2("Gravity", Math::ValuePtr(gravity), 0.1f))
				Scene::SetPhysicsWorldGravitty(gravity);

			static int32_t velocityIterations = Scene::GetPhysicsWorldVeloctiyIterations();
			if (Gui::DragInt("Velocity Iterations", &velocityIterations, 1.0f, 1, 100))
				Scene::SetPhysicsWorldVelocityIterations(velocityIterations);

			static int32_t positionIterations = Scene::GetPhysicsWorldPositionIterations();
			Gui::DragInt("Position Iterations", &positionIterations, 1.0f, 1, 100);
			Scene::SetPhysicsWorldPositionIterations(positionIterations);

			Gui::Checkbox("Show Physics Colliders", &m_Settings.ShowColliders);
		}

		if (selectedSetting == 2) // Editor Settings
		{
			Gui::DragFloat("Camera FOV", &m_Settings.EditorCameraFOV, 0.25f, 4.0f, 120.0f, "%.2f");

			Gui::Spacing();

			static const char* themes[] = {
				"Dark",
				"Light Gray",
				"Default",
				"Classic",
				"Light",
			};

			static const char* currentTheme = themes[0];

			if (Gui::BeginCombo("Editor Theme", currentTheme))
			{
				for (uint32_t i = 0; i < SP_ARRAYCOUNT(themes); i++)
				{
					bool isSelected = strcmp(currentTheme, themes[i]) == 0;
					if (Gui::Selectable(themes[i], isSelected))
					{
						currentTheme = themes[i];

						if (currentTheme == themes[0])
							Application::Get().GetGuiLayer()->SetDarkThemeColors();
						if (currentTheme == themes[1])
							Application::Get().GetGuiLayer()->SetLightGrayThemeColors();
						if (currentTheme == themes[2])
							Gui::StyleColorsDark();
						if (currentTheme == themes[3])
							Gui::StyleColorsClassic();
						if (currentTheme == themes[4])
							Gui::StyleColorsLight();
					}

					if (isSelected)
						Gui::SetItemDefaultFocus();
				}

				Gui::EndMenu();
			}

			ImGuiIO& io = Gui::GetIO();
			ImFont* currentFont = Gui::GetFont();
			if (Gui::BeginCombo("Editor Font", currentFont->GetDebugName()))
			{
				for (uint32_t i = 0; i < io.Fonts->Fonts.Size; i++)
				{
					ImFont* font = io.Fonts->Fonts[i];
					Gui::PushID((void*)font);
					if (Gui::Selectable(font->GetDebugName(), font == currentFont))
						io.FontDefault = font;
					Gui::PopID();
				}
				Gui::EndCombo();
			}

			static bool lockEditorCameraRotation = false;
			if (Gui::Checkbox("Lock Camera Rotation", &lockEditorCameraRotation))
				EditorCamera::LockCameraRotation(lockEditorCameraRotation);
		}

		if (selectedSetting == 3) // Runtime Settings
		{

		}

		if (selectedSetting == 4) // Build Settings
		{
			char buffer[256];
			strcpy(buffer, m_Settings.CurrentEditorScenePath.string().c_str());
			Gui::InputText("Active Scene", buffer, 256, ImGuiInputTextFlags_ReadOnly);

			if (Gui::Button("Build and Run"))
				m_Settings.LaunchRuntimeCallback();
		}
	}

}
