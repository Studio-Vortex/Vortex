#include "ProjectSettingsPanel.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"

namespace Vortex {

	ProjectSettingsPanel::ProjectSettingsPanel(const SharedRef<Project>& project)
		: m_Properties(project->GetProperties()) { }

	void ProjectSettingsPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Project Settings", &s_ShowPanel);

			if (Gui::BeginTabBar("##Tabs"))
			{
				if (Gui::BeginTabItem("General"))
				{
					UI::BeginPropertyGrid();

					std::string& projectName = m_Properties.General.Name;
					UI::Property("Project Name", projectName);

					std::filesystem::path& assetDirectory = m_Properties.General.AssetDirectory;
					std::string assetDirectoryStr = assetDirectory.string();
					UI::Property("Asset Directory", assetDirectoryStr, true);

					std::filesystem::path& startScene = m_Properties.General.StartScene;
					std::string startSceneStr = startScene.string();
					if (UI::Property("Start Scene", startSceneStr, true))
						m_Properties.General.StartScene = startSceneStr;

					UI::EndPropertyGrid();

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Physics"))
				{
					if (Gui::TreeNodeEx("3D", treeNodeFlags))
					{
						UI::BeginPropertyGrid();

						UI::Property("Collider Color", &m_Properties.PhysicsProps.Physics3DColliderColor);

						Math::vec3 gravity3D = Physics::GetPhysicsSceneGravity();
						if (UI::Property("Gravity", gravity3D))
							Physics::SetPhysicsSceneGravity(gravity3D);

						UI::EndPropertyGrid();

						static const char* broadphaseTypes[] = {
							"Sweep And Prune",
							"Multi Box Prune",
							"Automatic Box Prune",
						};

						static const char* currentBroadphaseType = broadphaseTypes[(uint32_t)m_Properties.PhysicsProps.BroadphaseModel];

						if (Gui::BeginCombo("Broadphase Model", currentBroadphaseType))
						{
							uint32_t arraySize = VX_ARRAYCOUNT(broadphaseTypes);

							for (uint32_t i = 0; i < arraySize; i++)
							{
								bool isSelected = strcmp(currentBroadphaseType, broadphaseTypes[i]) == 0;
								if (Gui::Selectable(broadphaseTypes[i], isSelected))
								{
									currentBroadphaseType = broadphaseTypes[i];

									if (i == 0)
										m_Properties.PhysicsProps.BroadphaseModel = BroadphaseType::SweepAndPrune;
									if (i == 1)
										m_Properties.PhysicsProps.BroadphaseModel = BroadphaseType::MultiBoxPrune;
									if (i == 2)
										m_Properties.PhysicsProps.BroadphaseModel = BroadphaseType::AutomaticBoxPrune;
								}

								if (isSelected)
									Gui::SetItemDefaultFocus();

								if (i != arraySize - 1)
									Gui::Separator();
							}

							Gui::EndMenu();
						}

						static const char* frictionTypes[] = {
							"Patch",
							"One Directional",
							"Two Directional",
						};

						static const char* currentFrictionType = frictionTypes[(int)m_Properties.PhysicsProps.FrictionModel];

						if (Gui::BeginCombo("Friction Model", currentFrictionType))
						{
							uint32_t arraySize = VX_ARRAYCOUNT(frictionTypes);

							for (uint32_t i = 0; i < arraySize; i++)
							{
								bool isSelected = strcmp(currentFrictionType, frictionTypes[i]) == 0;
								if (Gui::Selectable(frictionTypes[i], isSelected))
								{
									currentFrictionType = frictionTypes[i];

									if (i == 0)
										m_Properties.PhysicsProps.FrictionModel = FrictionType::Patch;
									if (i == 1)
										m_Properties.PhysicsProps.FrictionModel = FrictionType::OneDirectional;
									if (i == 2)
										m_Properties.PhysicsProps.FrictionModel = FrictionType::TwoDirectional;
								}

								if (isSelected)
									Gui::SetItemDefaultFocus();

								if (i != arraySize - 1)
									Gui::Separator();
							}

							Gui::EndMenu();
						}

						UI::BeginPropertyGrid();

						int32_t positionIterations3D = Physics::GetPhysicsScenePositionIterations();
						if (UI::Property("Position Iterations", positionIterations3D, 1.0f, 1, 100))
							Physics::SetPhysicsScenePositionIterations(positionIterations3D);

						int32_t velocityIterations3D = Physics::GetPhysicsSceneVelocityIterations();
						if (UI::Property("Velocity Iterations", velocityIterations3D, 1.0f, 1, 100))
							Physics::SetPhysicsSceneVelocityIterations(velocityIterations3D);

						UI::EndPropertyGrid();

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("2D", treeNodeFlags))
					{
						UI::BeginPropertyGrid();

						UI::Property("Collider Color", &m_Properties.PhysicsProps.Physics2DColliderColor);

						Math::vec2 gravity2D = Physics2D::GetPhysicsWorldGravity();
						if (UI::Property("Gravity", gravity2D))
							Physics2D::SetPhysicsWorldGravitty(gravity2D);


						int32_t positionIterations2D = Physics2D::GetPhysicsWorldPositionIterations();
						if (UI::Property("Position Iterations", positionIterations2D, 1.0f, 1, 100))
							Physics2D::SetPhysicsWorldPositionIterations(positionIterations2D);

						int32_t velocityIterations2D = Physics2D::GetPhysicsWorldVelocityIterations();
						if (UI::Property("Velocity Iterations", velocityIterations2D, 1.0f, 1, 100))
							Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations2D);

						UI::EndPropertyGrid();

						Gui::TreePop();
					}

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Scripting"))
				{
					UI::BeginPropertyGrid();

					std::filesystem::path& scriptBinaryPath = m_Properties.ScriptingProps.ScriptBinaryPath;
					std::string scriptBinaryPathStr = scriptBinaryPath.string();
					UI::Property("Script Binary Path", scriptBinaryPathStr, true);

					UI::Property("Enable Debugging", m_Properties.ScriptingProps.EnableMonoDebugging);
					UI::Property("Reload Assembly On Play", m_Properties.ScriptingProps.ReloadAssemblyOnPlay);

					UI::EndPropertyGrid();

					Gui::EndTabItem();
				}

				if (Gui::BeginTabItem("Editor"))
				{
					if (Gui::TreeNodeEx("Preferences", treeNodeFlags))
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

									if (i == 0)
										Application::Get().GetGuiLayer()->SetDarkThemeColors();
									if (i == 1)
										Application::Get().GetGuiLayer()->SetLightGrayThemeColors();
									if (i == 2)
										Gui::StyleColorsDark();
									if (i == 3)
										Gui::StyleColorsClassic();
									if (i == 4)
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

						UI::BeginPropertyGrid();

						UI::Property("Frame Step Count", m_Properties.EditorProps.FrameStepCount);
						UI::Property("Draw Editor Grid", m_Properties.EditorProps.DrawEditorGrid);
						UI::Property("Draw Editor Axes", m_Properties.EditorProps.DrawEditorAxes);

						UI::EndPropertyGrid();

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Editor Camera", treeNodeFlags))
					{
						UI::BeginPropertyGrid();

						Math::vec3 moveSpeed = EditorCamera::GetMoveSpeed();
						if (UI::Property("Move Speed", moveSpeed))
							EditorCamera::SetMoveSpeed(moveSpeed);

						UI::Property("Camera FOV", m_Properties.EditorProps.EditorCameraFOV, 0.25f, 4.0f, 120.0f);

						UI::EndPropertyGrid();

						Gui::TreePop();
					}

					if (Gui::TreeNodeEx("Gizmos", treeNodeFlags))
					{
						UI::BeginPropertyGrid();

						// Minimums don't work here for some reason
						UI::Property("Enabled", m_Properties.GizmoProps.Enabled);
						UI::Property("Orthographic Gizmos", m_Properties.GizmoProps.IsOrthographic);
						UI::Property("Snap", m_Properties.GizmoProps.SnapEnabled);
						UI::Property("Snap Value", m_Properties.GizmoProps.SnapValue, 0.05f, 0.05f);
						UI::Property("Rotation Snap Value", m_Properties.GizmoProps.RotationSnapValue, 1.0f, 1.0f);
						UI::Property("Draw Grid", m_Properties.GizmoProps.DrawGrid);
						if (m_Properties.GizmoProps.DrawGrid)
							UI::Property("Grid Size", m_Properties.GizmoProps.GridSize, 0.5f, 0.5f);

						UI::EndPropertyGrid();

						Gui::TreePop();
					}

					Gui::EndTabItem();
				}

				Gui::EndTabBar();
			}

			Gui::End();
		}
	}

}
