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

			if (Gui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
			{
				if (Gui::BeginTabItem("Renderer 2D"))
				{
					if (Gui::ColorEdit3("Clear Color", Math::ValuePtr(m_Settings.ClearColor)))
						RenderCommand::SetClearColor(m_Settings.ClearColor);

					float lineWidth = Renderer2D::GetLineWidth();
					if (Gui::DragFloat("Line Width", &lineWidth, 0.1f, 0.1f, 4.0f, "%.2f"))
						Renderer2D::SetLineWidth(lineWidth);

					RendererAPI::TriangleCullMode cullMode = Renderer2D::GetCullMode();
					static const char* cullModes[4] = {
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::None),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::Front),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::Back),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::FrontAndBack)
					};

					static const char* currentCullMode = Utils::TriangleCullModeToString(cullMode);

					if (Gui::BeginCombo("Cull Mode", currentCullMode))
					{
						for (uint32_t i = 0; i < SP_ARRAYCOUNT(cullModes); i++)
						{
							bool isSelected = strcmp(currentCullMode, cullModes[i]) == 0;
							if (Gui::Selectable(cullModes[i], isSelected))
							{
								currentCullMode = cullModes[i];

								if (currentCullMode == cullModes[0])
									Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::None);
								if (currentCullMode == cullModes[1])
									Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::Front);
								if (currentCullMode == cullModes[2])
									Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::Back);
								if (currentCullMode == cullModes[3])
									Renderer2D::SetCullMode(RendererAPI::TriangleCullMode::FrontAndBack);
							}

							if (isSelected)
								Gui::SetItemDefaultFocus();
						}

						Gui::EndMenu();
					}

					static bool wireframeMode = false;
					if (Gui::Checkbox("Show Wireframe", &wireframeMode))
						RenderCommand::SetWireframe(wireframeMode);

					static bool vsync = true;
					if (Gui::Checkbox("Use VSync", &vsync))
						Application::Get().GetWindow().SetVSync(vsync);

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Physics 2D"))
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

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Editor"))
				{
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

					Gui::Spacing();

					// Editor Camera Settings
					Gui::Text("Camera");
					Gui::Separator();
					Gui::Spacing();

					Math::vec3 moveSpeed = EditorCamera::GetMoveSpeed();
					if (Gui::DragFloat3("Move Speed", Math::ValuePtr(moveSpeed), 0.01f, 0.01f, 1.0f, "%.2f"))
						EditorCamera::SetMoveSpeed(moveSpeed);

					Gui::DragFloat("Camera FOV", &m_Settings.EditorCameraFOV, 0.25f, 4.0f, 120.0f, "%.2f");

					static bool lockEditorCameraRotation = false;
					if (Gui::Checkbox("Lock Camera Rotation", &lockEditorCameraRotation))
						EditorCamera::LockCameraRotation(lockEditorCameraRotation);

					Gui::Spacing();

					// Gizmo Settings
					Gui::Text("Gizmos");
					Gui::Separator();
					Gui::Spacing();

					// Minimums don't work here for some reason
					Gui::Checkbox("Enabled", &m_Settings.Gizmos.Enabled);
					Gui::Checkbox("Orthographic Gizmos", &m_Settings.Gizmos.IsOrthographic);
					Gui::Checkbox("Snap", &m_Settings.Gizmos.SnapEnabled);
					Gui::DragFloat("Snap Value", &m_Settings.Gizmos.SnapValue, 0.05f, 0.05f, 0.0f, "%.2f");
					Gui::DragFloat("Rotation Snap Value", &m_Settings.Gizmos.RotationSnapValue, 1.0f, 1.0f, 0.0f, "%.2f");
					Gui::Checkbox("Draw Grid", &m_Settings.Gizmos.DrawGrid);
					if (m_Settings.Gizmos.DrawGrid)
						Gui::DragFloat("Grid Size", &m_Settings.Gizmos.GridSize, 0.5f, 0.5f, 0.0f, "%.2f");

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Runtime"))
				{

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Build"))
				{
					char buffer[256];
					strcpy(buffer, m_Settings.CurrentEditorScenePath.string().c_str());
					Gui::InputText("Active Scene", buffer, 256, ImGuiInputTextFlags_ReadOnly);

					if (Gui::Button("Build and Run"))
						m_Settings.LaunchRuntimeCallback();

					Gui::EndTabItem();
				}

				Gui::EndTabBar();
			}

			Gui::End();
		}
	}

}
