#pragma once

namespace Sparky {

	struct ProjectProperties
	{
		struct ScriptingProperties
		{
			std::string AssetDirectory = "";
			std::string CSharpAssemblyPath = "";
			bool EnableDebugging = false;
		} ScriptingProps;
		
		struct RendererProperties
		{
			Math::vec3 ClearColor;
		} RendererProps;

		struct PhysicsProperties
		{
			Math::vec4 Physics3DColliderColor;
			Math::vec4 Physics2DColliderColor;
			bool ShowColliders;
		} PhysicsProps;

		struct EditorProperties
		{
			uint32_t FrameStepCount;
			bool DrawEditorGrid;
			bool DrawEditorAxes;
			float EditorCameraFOV;
		} EditorProps;

		struct GizmoProperties
		{
			bool Enabled;
			bool IsOrthographic;
			bool SnapEnabled;
			float SnapValue;
			float RotationSnapValue;
			bool DrawGrid;
			float GridSize;
		} GizmoProps;
	};

	class Project
	{
	public:
		Project(const ProjectProperties& props);
		~Project() = default;

		inline ProjectProperties& GetProperties() { return m_Properties; }
		inline const ProjectProperties& GetProperties() const { return m_Properties; }

		static SharedRef<Project> Create(const ProjectProperties& props);
	private:
		ProjectProperties m_Properties;
	};

}
