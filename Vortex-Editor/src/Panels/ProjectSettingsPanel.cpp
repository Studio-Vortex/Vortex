#include "ProjectSettingsPanel.h"

namespace Vortex {

	ProjectSettingsPanel::ProjectSettingsPanel(SharedReference<Project> project)
		: m_ProjectProperties(project->GetProperties())
	{
		m_CallbackPanels = std::vector<std::function<void()>>
		{
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderGeneralSettingsPanel),
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderEditorSettingsPanel),
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderGizmoSettingsPanel),
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderNetworkSettingsPanel),
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderPhysicsSettingsPanel),
			VX_BIND_CALLBACK(ProjectSettingsPanel::RenderScriptingSettingsPanel),
		};
	}

	void ProjectSettingsPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);
		Gui::BeginGroup();

		ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		Gui::BeginChild("Selections", { contentRegionAvail.x / 4.0f, contentRegionAvail.y }, true);

		static const char* selections[] = { "General", "Editor", "Gizmo", "Network", "Physics", "Scripting" };
		static auto count = VX_ARRAYSIZE(selections);
		static uint32_t selection = 0;

		for (uint32_t i = 0; i < count; i++)
		{
			if (Gui::Selectable(selections[i], selection == i))
			{
				selection = i;
			}

			UI::Draw::Underline();
		}

		Gui::EndChild();
		Gui::EndGroup();

		Gui::SameLine();
		Gui::BeginChild("Right", Gui::GetContentRegionAvail());

		m_CallbackPanels[selection]();

		Gui::EndChild();

		Gui::End();
	}

	void ProjectSettingsPanel::RenderGeneralSettingsPanel()
	{
		UI::BeginPropertyGrid();

		UI::Property("Project Name", m_ProjectProperties.General.Name);

		const Fs::Path& assetDirectory = m_ProjectProperties.General.AssetDirectory;
		std::string assetDirectoryStr = assetDirectory.string();
		UI::Property("Asset Directory", assetDirectoryStr, true);

		const Fs::Path& assetRegistry = m_ProjectProperties.General.AssetRegistryPath;
		std::string assetRegistryStr = assetRegistry.string();
		UI::Property("Asset Registry", assetRegistryStr, true);

		const Fs::Path& startScene = m_ProjectProperties.General.StartScene;
		std::string startSceneStr = startScene.string();
		UI::Property("Start Scene", startSceneStr, true);

		UI::EndPropertyGrid();
	}

	void ProjectSettingsPanel::RenderEditorSettingsPanel()
	{
		UI::BeginPropertyGrid();

		enum class Theme { Dark, LightGray, Default, Classic, Light };
		static const char* themes[] = { "Dark", "Light Gray", "Default", "Classic", "Light" };
		static int32_t currentTheme = (int32_t)Theme::Dark;
		if (UI::PropertyDropdown("Editor Theme", themes, VX_ARRAYSIZE(themes), currentTheme))
		{
			switch (currentTheme)
			{
				case 0: Application::Get().GetGuiLayer()->SetDarkThemeColors(); break;
				case 1: Application::Get().GetGuiLayer()->SetLightGrayThemeColors(); break;
				case 2: Gui::StyleColorsDark(); break;
				case 3: Gui::StyleColorsClassic(); break;
				case 4: Gui::StyleColorsLight(); break;
			}
		}

		const ImGuiIO& io = Gui::GetIO();
		std::vector<const char*> buffer;
		const uint32_t fontCount = io.Fonts->Fonts.Size;

		for (uint32_t i = 0; i < fontCount; i++)
		{
			buffer.push_back(io.Fonts->Fonts[i]->GetDebugName());
		}

		ImFont* currentFont = Gui::GetFont();
		UI::FontSelector("Editor Font", buffer.data(), fontCount, currentFont);

		UI::Property("Frame Step Count", m_ProjectProperties.EditorProps.FrameStepCount);
		UI::Property("Draw Editor Grid", m_ProjectProperties.EditorProps.DrawEditorGrid);
		UI::Property("Draw Editor Axes", m_ProjectProperties.EditorProps.DrawEditorAxes);

		UI::EndPropertyGrid();
	}

	void ProjectSettingsPanel::RenderGizmoSettingsPanel()
	{
		UI::BeginPropertyGrid();

		UI::Property("Enabled", m_ProjectProperties.GizmoProps.Enabled);
		UI::Property("Orthographic Gizmos", m_ProjectProperties.GizmoProps.IsOrthographic);
		UI::Property("Enable Snapping", m_ProjectProperties.GizmoProps.SnapEnabled);
		UI::Property("Snap Value", m_ProjectProperties.GizmoProps.SnapValue, 0.05f, FLT_MIN, FLT_MAX);
		UI::Property("Rotation Snap Value", m_ProjectProperties.GizmoProps.RotationSnapValue, 1.0f, FLT_MIN, FLT_MAX);
		UI::Property("Gizmo Size", m_ProjectProperties.GizmoProps.GizmoSize, 0.05f, FLT_MIN, FLT_MAX);
		UI::Property("Draw Grid", m_ProjectProperties.GizmoProps.DrawGrid);
		if (m_ProjectProperties.GizmoProps.DrawGrid)
		{
			UI::Property("Grid Size", m_ProjectProperties.GizmoProps.GridSize, 0.5f, FLT_MIN, FLT_MAX);
		}

		UI::EndPropertyGrid();
	}

	void ProjectSettingsPanel::RenderPhysicsSettingsPanel()
	{
		if (UI::PropertyGridHeader("3D", true))
		{
			UI::BeginPropertyGrid();

			UI::Property("Collider Color", &m_ProjectProperties.PhysicsProps.Physics3DColliderColor);

			Math::vec3 gravity3D = Physics::GetPhysicsSceneGravity();
			if (UI::Property("Gravity", gravity3D))
			{
				Physics::SetPhysicsSceneGravity(gravity3D);

				// we need to wake up all the actors because a number of them could be sleeping
				Physics::WakeUpActors();
			}

			static const char* broadphaseTypes[] = { "Sweep And Prune", "Multi Box Pruning", "Automatic Box Pruning" };
			int32_t currentBroadphaseType = (int32_t)m_ProjectProperties.PhysicsProps.BroadphaseModel;
			if (UI::PropertyDropdown("Broadphase Model", broadphaseTypes, VX_ARRAYSIZE(broadphaseTypes), currentBroadphaseType))
			{
				m_ProjectProperties.PhysicsProps.BroadphaseModel = (BroadphaseType)currentBroadphaseType;
			}

			static const char* frictionTypes[3] = { "Patch", "One Directional", "Two Directional" };
			int32_t currentFrictionType = (int32_t)m_ProjectProperties.PhysicsProps.FrictionModel;
			if (UI::PropertyDropdown("Friction Model", frictionTypes, VX_ARRAYSIZE(frictionTypes), currentFrictionType))
			{
				m_ProjectProperties.PhysicsProps.FrictionModel = (FrictionType)currentFrictionType;
			}

			int32_t positionIterations3D = Physics::GetPhysicsScenePositionIterations();
			if (UI::Property("Position Iterations", positionIterations3D, 1, 1, 256))
			{
				Physics::SetPhysicsScenePositionIterations(positionIterations3D);
			}

			int32_t velocityIterations3D = Physics::GetPhysicsSceneVelocityIterations();
			if (UI::Property("Velocity Iterations", velocityIterations3D, 1, 1, 256))
			{
				Physics::SetPhysicsSceneVelocityIterations(velocityIterations3D);
			}

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}

		if (UI::PropertyGridHeader("2D", true))
		{
			UI::BeginPropertyGrid();

			UI::Property("Collider Color", &m_ProjectProperties.PhysicsProps.Physics2DColliderColor);

			Math::vec2 gravity2D = Physics2D::GetPhysicsWorldGravity();
			if (UI::Property("Gravity", gravity2D))
			{
				Physics2D::SetPhysicsWorldGravitty(gravity2D);
			}

			int32_t positionIterations2D = Physics2D::GetPhysicsWorldPositionIterations();
			if (UI::Property("Position Iterations", positionIterations2D, 1.0f, 1, 100))
			{
				Physics2D::SetPhysicsWorldPositionIterations(positionIterations2D);
			}

			int32_t velocityIterations2D = Physics2D::GetPhysicsWorldVelocityIterations();
			if (UI::Property("Velocity Iterations", velocityIterations2D, 1.0f, 1, 100))
			{
				Physics2D::SetPhysicsWorldVelocityIterations(velocityIterations2D);
			}

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}
	}

	void ProjectSettingsPanel::RenderNetworkSettingsPanel()
	{
		UI::BeginPropertyGrid();



		UI::EndPropertyGrid();
	}

	void ProjectSettingsPanel::RenderScriptingSettingsPanel()
	{
		UI::BeginPropertyGrid();

		const Fs::Path& scriptBinaryPath = m_ProjectProperties.ScriptingProps.ScriptBinaryPath;
		std::string scriptBinaryPathStr = scriptBinaryPath.string();
		UI::Property("Script Binary Path", scriptBinaryPathStr, true);
		UI::Property("Debug Listener Port", m_ProjectProperties.ScriptingProps.DebugListenerPort);
		UI::Property("Enable Debugging", m_ProjectProperties.ScriptingProps.EnableMonoDebugging);
		UI::Property("Reload Assembly On Play", m_ProjectProperties.ScriptingProps.ReloadAssemblyOnPlay);

		UI::EndPropertyGrid();
	}

}
