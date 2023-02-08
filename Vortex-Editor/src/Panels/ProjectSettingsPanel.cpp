#include "ProjectSettingsPanel.h"

#include <Vortex/Physics/3D/PhysXAPIHelpers.h>

namespace Vortex {

	ProjectSettingsPanel::ProjectSettingsPanel(SharedRef<Project> project)
		: m_Properties(project->GetProperties()) { }

	void ProjectSettingsPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		if (!s_ShowPanel)
			return;

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
				if (UI::PropertyGridHeader("3D", false))
				{
					UI::BeginPropertyGrid();

					UI::Property("Collider Color", &m_Properties.PhysicsProps.Physics3DColliderColor);

					Math::vec3 gravity3D = Physics::GetPhysicsSceneGravity();
					if (UI::Property("Gravity", gravity3D))
					{
						Physics::SetPhysicsSceneGravity(gravity3D);
						Physics::WakeUpActors();
					}

					static const char* broadphaseTypes[] = { "Sweep And Prune", "Multi Box Prune", "Automatic Box Prune" };
					int32_t currentBroadphaseType = (int32_t)m_Properties.PhysicsProps.BroadphaseModel;
					if (UI::PropertyDropdown("Broadphase Model", broadphaseTypes, VX_ARRAYCOUNT(broadphaseTypes), currentBroadphaseType))
						m_Properties.PhysicsProps.BroadphaseModel = (BroadphaseType)currentBroadphaseType;

					static const char* frictionTypes[3] = { "Patch", "One Directional", "Two Directional" };
					int32_t currentFrictionType = (int32_t)m_Properties.PhysicsProps.FrictionModel;
					if (UI::PropertyDropdown("Friction Model", frictionTypes, VX_ARRAYCOUNT(frictionTypes), currentFrictionType))
						m_Properties.PhysicsProps.FrictionModel = (FrictionType)currentFrictionType;

					int32_t positionIterations3D = Physics::GetPhysicsScenePositionIterations();
					if (UI::Property("Position Iterations", positionIterations3D, 1.0f, 1, 100))
						Physics::SetPhysicsScenePositionIterations(positionIterations3D);

					int32_t velocityIterations3D = Physics::GetPhysicsSceneVelocityIterations();
					if (UI::Property("Velocity Iterations", velocityIterations3D, 1.0f, 1, 100))
						Physics::SetPhysicsSceneVelocityIterations(velocityIterations3D);

					UI::EndPropertyGrid();
					UI::EndTreeNode();
				}

				if (UI::PropertyGridHeader("2D", false))
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
					UI::EndTreeNode();
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
				if (UI::PropertyGridHeader("Preferences", false))
				{
					UI::BeginPropertyGrid();

					enum class Theme { Dark, LightGray, Default, Classic, Light };
					static const char* themes[] = { "Dark", "Light Gray", "Default", "Classic", "Light" };
					static int32_t currentTheme = (int32_t)Theme::Dark;
					if (UI::PropertyDropdown("Editor Theme", themes, VX_ARRAYCOUNT(themes), currentTheme))
					{
						if (currentTheme == 0)
							Application::Get().GetGuiLayer()->SetDarkThemeColors();
						if (currentTheme == 1)
							Application::Get().GetGuiLayer()->SetLightGrayThemeColors();
						if (currentTheme == 2)
							Gui::StyleColorsDark();
						if (currentTheme == 3)
							Gui::StyleColorsClassic();
						if (currentTheme == 4)
							Gui::StyleColorsLight();
					}

					std::vector<const char*> buffer;
					uint32_t count = io.Fonts->Fonts.Size;
					for (uint32_t i = 0; i < count; i++)
					{
						buffer.push_back(io.Fonts->Fonts[i]->GetDebugName());
					}

					ImFont* currentFont = Gui::GetFont();
					UI::FontSelector("Editor Font", buffer.data(), count, currentFont);

					UI::Property("Frame Step Count", m_Properties.EditorProps.FrameStepCount);
					UI::Property("Draw Editor Grid", m_Properties.EditorProps.DrawEditorGrid);
					UI::Property("Draw Editor Axes", m_Properties.EditorProps.DrawEditorAxes);

					UI::EndPropertyGrid();
					UI::EndTreeNode();
				}

				if (UI::PropertyGridHeader("Gizmos", false))
				{
					UI::BeginPropertyGrid();

					// Minimums don't work here for some reason
					UI::Property("Enabled", m_Properties.GizmoProps.Enabled);
					UI::Property("Orthographic Gizmos", m_Properties.GizmoProps.IsOrthographic);
					UI::Property("Snap", m_Properties.GizmoProps.SnapEnabled);
					UI::Property("Snap Value", m_Properties.GizmoProps.SnapValue, 0.05f, 0.05f);
					UI::Property("Rotation Snap Value", m_Properties.GizmoProps.RotationSnapValue, 1.0f, 1.0f);
					UI::Property("Gizmo Size", m_Properties.GizmoProps.GizmoSize, 0.05f, 0.05f);
					UI::Property("Draw Grid", m_Properties.GizmoProps.DrawGrid);
					if (m_Properties.GizmoProps.DrawGrid)
						UI::Property("Grid Size", m_Properties.GizmoProps.GridSize, 0.5f, 0.5f);

					UI::EndPropertyGrid();
					UI::EndTreeNode();
				}

				Gui::EndTabItem();
			}

			Gui::EndTabBar();
		}

		Gui::End();
	}

    void ProjectSettingsPanel::SetProjectContext(SharedRef<Project> project)
    {
		m_Properties = project->GetProperties();
    }

}
