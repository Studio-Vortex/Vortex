#include "vxpch.h"
#include "ProjectSerializer.h"

#include "Vortex/Utils/YAML_SerializationUtils.h"
#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/Renderer.h"

#include <fstream>

namespace Vortex {

	ProjectSerializer::ProjectSerializer(const SharedRef<Project>& project)
		: m_Project(project) { }

	bool ProjectSerializer::Serialize(const std::filesystem::path& path)
	{
		const ProjectProperties& props = m_Project->GetProperties();

		YAML::Emitter out;

		out << YAML::BeginMap; // Project Properties
		out << YAML::Key << "Project" << YAML::Value << YAML::BeginMap; // Project

		{
			out << YAML::Key << "General" << YAML::BeginMap; // General
			{
				out << YAML::Key << "Name" << YAML::Value << props.General.Name;
				out << YAML::Key << "AssetDirectory" << YAML::Value << props.General.AssetDirectory.string();
				out << YAML::Key << "ScriptBinaryPath" << YAML::Value << props.General.ScriptBinaryPath.string();
				out << YAML::Key << "StartScene" << YAML::Value << props.General.StartScene.string();
				out << YAML::Key << "EnableMonoDebugging" << YAML::Value << props.General.EnableMonoDebugging;
			}
			out << YAML::EndMap; // General

			out << YAML::Key << "RendererProperties" << YAML::BeginMap; // RendererProperties
			{
				out << YAML::Key << "DisplaySceneIconsInEditor" << YAML::Value << props.RendererProps.DisplaySceneIconsInEditor;
				out << YAML::Key << "TriangleCullMode" << YAML::Value << props.RendererProps.TriangleCullMode;
			}
			out << YAML::EndMap; // RendererProperties

			out << YAML::Key << "PhysicsProperties" << YAML::BeginMap; // Physics Properties
			{
				out << YAML::Key << "Physics2DColliderColor" << YAML::Value << props.PhysicsProps.Physics2DColliderColor;
				out << YAML::Key << "Physics3DColliderColor" << YAML::Value << props.PhysicsProps.Physics3DColliderColor;
				out << YAML::Key << "ShowPhysicsColliders" << YAML::Value << props.PhysicsProps.ShowColliders;
			}
			out << YAML::EndMap; // Physics Properties

			out << YAML::Key << "EditorProperties" << YAML::BeginMap; // Editior Properties
			{
				out << YAML::Key << "DrawGridAxes" << YAML::Value << props.EditorProps.DrawEditorAxes;
				out << YAML::Key << "DrawGrid" << YAML::Value << props.EditorProps.DrawEditorGrid;
				out << YAML::Key << "EditorCameraFOV" << YAML::Value << props.EditorProps.EditorCameraFOV;
				out << YAML::Key << "MaximizeOnPlay" << YAML::Value << props.EditorProps.MaximizeOnPlay;
				out << YAML::Key << "MuteAudioSources" << YAML::Value << props.EditorProps.MuteAudioSources;
				out << YAML::Key << "ReloadAssemblyOnPlay" << YAML::Value << props.EditorProps.ReloadAssemblyOnPlay;
				out << YAML::Key << "FrameStepCount" << YAML::Value << props.EditorProps.FrameStepCount;
			}
			out << YAML::EndMap; // Editior Properties

			out << YAML::Key << "GizmoProperties" << YAML::BeginMap; // Gizmo Properties
			{
				out << YAML::Key << "DrawGrid" << YAML::Value << props.GizmoProps.DrawGrid;
				out << YAML::Key << "Enabled" << YAML::Value << props.GizmoProps.Enabled;
				out << YAML::Key << "GridSize" << YAML::Value << props.GizmoProps.GridSize;
				out << YAML::Key << "IsOrthographic" << YAML::Value << props.GizmoProps.IsOrthographic;
				out << YAML::Key << "RotationSnapValue" << YAML::Value << props.GizmoProps.RotationSnapValue;
				out << YAML::Key << "SnapEnabled" << YAML::Value << props.GizmoProps.SnapEnabled;
				out << YAML::Key << "SnapValue" << YAML::Value << props.GizmoProps.SnapValue;
			}
			out << YAML::EndMap; // Gizmo Properties
		}

		out << YAML::EndMap; // Project
		out << YAML::EndMap; // Project Properties

		std::ofstream fout(path);
		fout << out.c_str();

		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& path)
	{
		ProjectProperties& props = m_Project->GetProperties();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(path.string());
		}
		catch (YAML::ParserException& e)
		{
			VX_CORE_ERROR("Failed to load project file '{}'\n     {}", path, e.what());
			return false;
		}

		auto projectData = data["Project"];

		if (!projectData)
			return false;

		{
			auto generalData = projectData["General"];
			props.General.AssetDirectory = generalData["AssetDirectory"].as<std::string>();
			props.General.EnableMonoDebugging = generalData["EnableMonoDebugging"].as<bool>();
			props.General.Name = generalData["Name"].as<std::string>();
			props.General.ScriptBinaryPath = generalData["ScriptBinaryPath"].as<std::string>();
			props.General.StartScene = generalData["StartScene"].as<std::string>();
		}

		{
			auto rendererData = projectData["RendererProperties"];
			props.RendererProps.DisplaySceneIconsInEditor = rendererData["DisplaySceneIconsInEditor"].as<bool>();
			if (rendererData["TriangleCullMode"])
			{
				props.RendererProps.TriangleCullMode = rendererData["TriangleCullMode"].as<std::string>();
				RendererAPI::TriangleCullMode cullMode = Utils::TriangleCullModeFromString(props.RendererProps.TriangleCullMode);
				Renderer::SetCullMode(cullMode);
			}
		}

		{
			auto physicsData = projectData["PhysicsProperties"];
			props.PhysicsProps.Physics2DColliderColor = physicsData["Physics2DColliderColor"].as<Math::vec4>();
			props.PhysicsProps.Physics3DColliderColor = physicsData["Physics3DColliderColor"].as<Math::vec4>();
			props.PhysicsProps.ShowColliders = physicsData["ShowPhysicsColliders"].as<bool>();
		}

		{
			auto editorData = projectData["EditorProperties"];
			props.EditorProps.DrawEditorAxes = editorData["DrawGridAxes"].as<bool>();
			props.EditorProps.DrawEditorGrid = editorData["DrawGrid"].as<bool>();
			props.EditorProps.EditorCameraFOV = editorData["EditorCameraFOV"].as<float>();
			props.EditorProps.MaximizeOnPlay = editorData["MaximizeOnPlay"].as<bool>();
			props.EditorProps.MuteAudioSources = editorData["MuteAudioSources"].as<bool>();
			props.EditorProps.ReloadAssemblyOnPlay = editorData["ReloadAssemblyOnPlay"].as<bool>();
			props.EditorProps.FrameStepCount = editorData["FrameStepCount"].as<uint32_t>();
		}

		{
			auto gizmoData = projectData["GizmoProperties"];
			props.GizmoProps.DrawGrid = gizmoData["DrawGrid"].as<bool>();
			props.GizmoProps.Enabled = gizmoData["Enabled"].as<bool>();
			props.GizmoProps.GridSize = gizmoData["GridSize"].as<float>();
			props.GizmoProps.IsOrthographic = gizmoData["IsOrthographic"].as<bool>();
			props.GizmoProps.RotationSnapValue = gizmoData["RotationSnapValue"].as<float>();
			props.GizmoProps.SnapEnabled = gizmoData["SnapEnabled"].as<bool>();
			props.GizmoProps.SnapValue = gizmoData["SnapValue"].as<float>();
		}

		return true;
	}

}
