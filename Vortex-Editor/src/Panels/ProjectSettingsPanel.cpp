#include "ProjectSettingsPanel.h"

namespace Vortex {
	
	ProjectSettingsPanel::ProjectSettingsPanel(const SharedRef<Project>& project)
		: m_Properties(project->GetProperties()) { }

	void ProjectSettingsPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Project Settings", &s_ShowPanel);

			if (Gui::BeginTabBar("##Tabs"))
			{
				if (Gui::BeginTabItem("General"))
				{
					char buffer[256];
					std::string& projectName = m_Properties.General.Name;
					memcpy(buffer, projectName.c_str(), projectName.size());
					buffer[projectName.size()] = '\0';

					if (Gui::InputText("Project Name", buffer, sizeof(buffer)))
					{
						projectName = std::string(buffer);
					}

					std::filesystem::path& assetDirectory = m_Properties.General.AssetDirectory;
					Gui::InputText("Asset Directory", (char*)assetDirectory.string().c_str(), assetDirectory.string().size(), ImGuiInputTextFlags_ReadOnly);

					std::filesystem::path& scriptBinPath = m_Properties.General.ScriptBinaryPath;
					Gui::InputText("Script Binary Path", (char*)scriptBinPath.string().c_str(), scriptBinPath.string().size(), ImGuiInputTextFlags_ReadOnly);

					std::filesystem::path& startScene = m_Properties.General.StartScene;
					Gui::InputText("Start Scene", (char*)startScene.string().c_str(), startScene.string().size(), ImGuiInputTextFlags_ReadOnly);

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Physics"))
				{
					Gui::PushFont(boldFont);
					Gui::Text("3D Physics");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					Gui::ColorEdit4("Collider Color", Math::ValuePtr(m_Properties.PhysicsProps.Physics3DColliderColor));

					static Math::vec3 gravity3D = Physics::GetPhysicsSceneGravity();
					if (Gui::DragFloat3("Gravity", Math::ValuePtr(gravity3D), 0.1f))
						Physics::SetPhysicsSceneGravity(gravity3D);

					static int32_t physicsPositionIterations = Physics::GetPhysicsScenePositionIterations();
					if (Gui::DragInt("Position Iterations", &physicsPositionIterations, 1.0f, 1, 100))
						Physics::SetPhysicsScenePositionIterations(physicsPositionIterations);

					static int32_t physicsVelocityIterations = Physics::GetPhysicsSceneVelocityIterations();
					if (Gui::DragInt("Velocity Iterations", &physicsVelocityIterations, 1.0f, 1, 100))
						Physics::SetPhysicsSceneVelocityIterations(physicsVelocityIterations);

					Gui::PushFont(boldFont);
					Gui::Text("2D Physics");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					Gui::ColorEdit4("Collider Color", Math::ValuePtr(m_Properties.PhysicsProps.Physics2DColliderColor));

					static Math::vec2 gravity2D = Physics2D::GetPhysicsWorldGravity();
					if (Gui::DragFloat2("Gravity", Math::ValuePtr(gravity2D), 0.1f))
						Physics2D::SetPhysicsWorldGravitty(gravity2D);

					static int32_t velocityIterations = Physics2D::GetPhysicsWorldVelocityIterations();
					if (Gui::DragInt("Velocity Iterations", &velocityIterations, 1.0f, 1, 100))
						Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations);

					static int32_t positionIterations = Physics2D::GetPhysicsWorldPositionIterations();
					Gui::DragInt("Position Iterations", &positionIterations, 1.0f, 1, 100);
					Physics2D::SetPhysicsWorldPositionIterations(positionIterations);

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Scripting"))
				{
					Gui::Checkbox("Enable Debugging", &m_Properties.General.EnableMonoDebugging);
					Gui::Checkbox("Reload Assembly On Play", &m_Properties.EditorProps.ReloadAssemblyOnPlay);

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
						uint32_t arraySize = VX_ARRAYCOUNT(themes);

						for (uint32_t i = 0; i < arraySize; i++)
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

							if (i != arraySize - 1)
								Gui::Separator();
						}

						Gui::EndMenu();
					}

					ImFont* currentFont = Gui::GetFont();
					if (Gui::BeginCombo("Editor Font", currentFont->GetDebugName()))
					{
						uint32_t arraySize = io.Fonts->Fonts.Size;

						for (uint32_t i = 0; i < arraySize; i++)
						{
							ImFont* font = io.Fonts->Fonts[i];
							Gui::PushID((void*)font);
							if (Gui::Selectable(font->GetDebugName(), font == currentFont))
								io.FontDefault = font;

							if (i != arraySize - 1)
								Gui::Separator();

							Gui::PopID();
						}

						Gui::EndCombo();
					}

					Gui::DragScalar("Frame Step Count", ImGuiDataType_U32, &m_Properties.EditorProps.FrameStepCount);
					Gui::Checkbox("Draw Editor Grid", &m_Properties.EditorProps.DrawEditorGrid);
					Gui::Checkbox("Draw Editor Axes", &m_Properties.EditorProps.DrawEditorAxes);

					Gui::Spacing();

					// Editor Camera Settings
					Gui::PushFont(boldFont);
					Gui::Text("Camera");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					Math::vec3 moveSpeed = EditorCamera::GetMoveSpeed();
					if (Gui::DragFloat3("Move Speed", Math::ValuePtr(moveSpeed), 0.01f, 0.01f, 1.0f, "%.2f"))
						EditorCamera::SetMoveSpeed(moveSpeed);

					Gui::DragFloat("Camera FOV", &m_Properties.EditorProps.EditorCameraFOV, 0.25f, 4.0f, 120.0f, "%.2f");

					Gui::Separator();
					Gui::Spacing();

					// Gizmo Settings
					Gui::PushFont(boldFont);
					Gui::Text("Gizmos");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					// Minimums don't work here for some reason
					Gui::Checkbox("Enabled", &m_Properties.GizmoProps.Enabled);
					Gui::Checkbox("Orthographic Gizmos", &m_Properties.GizmoProps.IsOrthographic);
					Gui::Checkbox("Snap", &m_Properties.GizmoProps.SnapEnabled);
					Gui::DragFloat("Snap Value", &m_Properties.GizmoProps.SnapValue, 0.05f, 0.05f, 0.0f, "%.2f");
					Gui::DragFloat("Rotation Snap Value", &m_Properties.GizmoProps.RotationSnapValue, 1.0f, 1.0f, 0.0f, "%.2f");
					Gui::Checkbox("Draw Grid", &m_Properties.GizmoProps.DrawGrid);
					if (m_Properties.GizmoProps.DrawGrid)
						Gui::DragFloat("Grid Size", &m_Properties.GizmoProps.GridSize, 0.5f, 0.5f, 0.0f, "%.2f");

					Gui::EndTabItem();
				}

				Gui::EndTabBar();
			}

			Gui::End();
		}
	}

}
