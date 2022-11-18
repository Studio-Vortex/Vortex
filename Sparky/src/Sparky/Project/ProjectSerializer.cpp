#include "sppch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Sparky {

	ProjectSerializer::ProjectSerializer(const SharedRef<Project>& project)
		: m_Project(project) { }

	void ProjectSerializer::Serialize(const std::string& filepath)
	{
		const ProjectProperties& props = m_Project->GetProperties();

		YAML::Emitter out;

		out << YAML::BeginMap;

		out << YAML::Key << "ProjectSettings" << YAML::BeginSeq;
		
		out << YAML::Key << "ScriptingProperties" << YAML::BeginSeq;
		out << YAML::Key << "AssetDirectory" << YAML::Value << props.ScriptingProps.AssetDirectory;
		out << YAML::Key << "CSharpAssemblyPath" << YAML::Value << props.ScriptingProps.CSharpAssemblyPath;
		out << YAML::Key << "EnableDebugging" << YAML::Value << props.ScriptingProps.EnableDebugging;
		out << YAML::EndSeq;

		out << YAML::Key << "RendererProperties" << YAML::BeginSeq;
		out << YAML::Key << "ClearColor" << YAML::Value << props.RendererProps.ClearColor;
		out << YAML::EndSeq;

		out << YAML::Key << "PhysicsProperties" << YAML::BeginSeq;
		out << YAML::Key << "Physics2DColliderColor" << YAML::Value << props.PhysicsProps.Physics2DColliderColor;
		out << YAML::Key << "Physics3DColliderColor" << YAML::Value << props.PhysicsProps.Physics3DColliderColor;
		out << YAML::Key << "ShowPhysicsColliders" << YAML::Value << props.PhysicsProps.ShowColliders;
		out << YAML::EndSeq;
		
		out << YAML::Key << "EditorProperties" << YAML::BeginSeq;
		out << YAML::Key << "DrawGridAxes" << YAML::Value << props.EditorProps.DrawEditorAxes;
		out << YAML::Key << "DrawGrid" << YAML::Value << props.EditorProps.DrawEditorGrid;
		out << YAML::Key << "EditorCameraFOV" << YAML::Value << props.EditorProps.EditorCameraFOV;
		out << YAML::Key << "FrameStepCount" << YAML::Value << props.EditorProps.FrameStepCount;
		out << YAML::EndSeq;
		
		out << YAML::Key << "GizmoProperties" << YAML::BeginSeq;
		out << YAML::Key << "DrawGrid" << YAML::Value << props.GizmoProps.DrawGrid;
		out << YAML::Key << "Enabled" << YAML::Value << props.GizmoProps.Enabled;
		out << YAML::Key << "GridSize" << YAML::Value << props.GizmoProps.GridSize;
		out << YAML::Key << "IsOrthographic" << YAML::Value << props.GizmoProps.IsOrthographic;
		out << YAML::Key << "RotationSnapValue" << YAML::Value << props.GizmoProps.RotationSnapValue;
		out << YAML::Key << "SnapEnabled" << YAML::Value << props.GizmoProps.SnapEnabled;
		out << YAML::Key << "SnapValue" << YAML::Value << props.GizmoProps.SnapValue;
		out << YAML::EndSeq;
		
		out << YAML::Key << "BuildProperties" << YAML::BeginSeq;
		out << YAML::EndSeq;

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void ProjectSerializer::SerializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
	}

	bool ProjectSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException& e)
		{
			SP_CORE_ERROR("Failed to load .sproject file '{}'\n     {}", filepath, e.what());
			return false;
		}

		if (!data["ProjectSettings"])
			return false;

		

		return true;
	}

	bool ProjectSerializer::DeserializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}
