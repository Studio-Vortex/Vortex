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

				if (Gui::BeginTabItem("Renderer"))
				{
					float lineWidth = Renderer2D::GetLineWidth();
					if (Gui::DragFloat("Line Width", &lineWidth, 0.1f, 0.1f, 4.0f, "%.2f"))
						Renderer2D::SetLineWidth(lineWidth);

					RendererAPI::TriangleCullMode cullMode = Renderer::GetCullMode();
					static const char* cullModes[4] = {
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::None),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::Front),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::Back),
						Utils::TriangleCullModeToString(RendererAPI::TriangleCullMode::FrontAndBack)
					};

					static const char* currentCullMode = Utils::TriangleCullModeToString(cullMode);

					SharedRef<Project> activeProject = Project::GetActive();
					ProjectProperties& projectProps = activeProject->GetProperties();

					if (Gui::BeginCombo("Cull Mode", currentCullMode))
					{
						uint32_t arraySize = VX_ARRAYCOUNT(cullModes);

						auto SetCullModeFunc = [&](RendererAPI::TriangleCullMode cullMode)
						{
							Renderer::SetCullMode(cullMode);
							projectProps.RendererProps.TriangleCullMode = Utils::TriangleCullModeToString(cullMode);
						};

						for (uint32_t i = 0; i < arraySize; i++)
						{
							bool isSelected = strcmp(currentCullMode, cullModes[i]) == 0;
							if (Gui::Selectable(cullModes[i], isSelected))
							{
								currentCullMode = cullModes[i];

								if (currentCullMode == cullModes[0])
									SetCullModeFunc(RendererAPI::TriangleCullMode::None);
								if (currentCullMode == cullModes[1])
									SetCullModeFunc(RendererAPI::TriangleCullMode::Front);
								if (currentCullMode == cullModes[2])
									SetCullModeFunc(RendererAPI::TriangleCullMode::Back);
								if (currentCullMode == cullModes[3])
									SetCullModeFunc(RendererAPI::TriangleCullMode::FrontAndBack);
							}

							if (isSelected)
								Gui::SetItemDefaultFocus();

							if (i != arraySize - 1)
								Gui::Separator();
						}

						Gui::EndMenu();
					}

					float sceneExposure = Renderer::GetSceneExposure();
					if (Gui::DragFloat("Scene Exposure", &sceneExposure, 0.01f, 0.01f, 1.0f, "%.2f"))
						Renderer::SetSceneExposure(sceneExposure);

					float gamma = Renderer::GetSceneGamma();
					if (Gui::DragFloat("Gamma", &gamma, 0.01f, 0.01f, 0.0f, "%.2f"))
						Renderer::SetSceneGamma(gamma);

					static bool wireframeMode = false;
					if (Gui::Checkbox("Show Wireframe", &wireframeMode))
						RenderCommand::SetWireframe(wireframeMode);

					static bool vsync = true;
					if (Gui::Checkbox("Enable VSync", &vsync))
						Application::Get().GetWindow().SetVSync(vsync);

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Physics"))
				{
					Gui::Checkbox("Show Physics Colliders", &m_Properties.PhysicsProps.ShowColliders);

					Gui::Spacing();

					Gui::PushFont(boldFont);
					Gui::Text("3D Physics");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					Gui::ColorEdit4("Collider Color", Math::ValuePtr(m_Properties.PhysicsProps.Physics3DColliderColor));

					static Math::vec3 gravity3D = Physics::GetPhysicsSceneGravity();
					if (Gui::DragFloat3("Gravity", Math::ValuePtr(gravity3D), 0.1f))
						Physics::SetPhysicsSceneGravitty(gravity3D);

					static int32_t physicsPositionIterations = Physics::GetPhysicsPositionIterations();
					if (Gui::DragInt("Position Iterations", &physicsPositionIterations, 1.0f, 1, 100))
						Physics::SetPhysicsScenePositionIterations(physicsPositionIterations);

					static int32_t physicsVelocityIterations = Physics::GetPhysicsVelocityIterations();
					if (Gui::DragInt("Velocity Iterations", &physicsVelocityIterations, 1.0f, 1, 100))
						Physics::SetPhysicsSceneVelocityIterations(physicsVelocityIterations);

					Gui::PushFont(boldFont);
					Gui::Text("2D Physics");
					Gui::PopFont();
					Gui::Separator();
					Gui::Spacing();

					Gui::ColorEdit4("Collider Color", Math::ValuePtr(m_Properties.PhysicsProps.Physics2DColliderColor));

					static Math::vec2 gravity2D = Physics2D::GetPhysicsWorld2DGravity();
					if (Gui::DragFloat2("Gravity", Math::ValuePtr(gravity2D), 0.1f))
						Physics2D::SetPhysicsWorldGravitty(gravity2D);

					static int32_t velocityIterations = Physics2D::GetPhysicsWorld2DVeloctiyIterations();
					if (Gui::DragInt("Velocity Iterations", &velocityIterations, 1.0f, 1, 100))
						Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations);

					static int32_t positionIterations = Physics2D::GetPhysicsWorld2DPositionIterations();
					Gui::DragInt("Position Iterations", &positionIterations, 1.0f, 1, 100);
					Physics2D::SetPhysicsWorldPositionIterations(positionIterations);

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

					static bool lockFacingForward = false;
					if (Gui::Checkbox("Lock Camera To 2D View", &lockFacingForward))
						EditorCamera::LockTo2DView(lockFacingForward);

					static bool lockFacingDown = false;
					if (Gui::Checkbox("Lock Camera To Top Down View", &lockFacingDown))
						EditorCamera::LockToTopDownView(lockFacingDown);

					Gui::Separator();

					Gui::Checkbox("Reload Assembly On Play", &m_Properties.EditorProps.ReloadAssemblyOnPlay);

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
