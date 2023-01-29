#include "vxpch.h"
#include "ProjectSerializer.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Utils/YAML_SerializationUtils.h"
#include "Vortex/Renderer/RendererAPI.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"

#include <fstream>

namespace Vortex {

	namespace Utils {

		static std::string BroadphaseTypeToString(BroadphaseType broadphaseModel)
		{
			switch (broadphaseModel)
			{
				case BroadphaseType::SweepAndPrune:     return "SweepAndPrune";
				case BroadphaseType::MultiBoxPrune:     return "MultiBoxPrune";
				case BroadphaseType::AutomaticBoxPrune: return "AutomaticBoxPrune";
			}

			VX_CORE_ASSERT(false, "Unknown Broadphase Type!");
			return {};
		}

		static BroadphaseType BroadphaseTypeFromString(const std::string& broadphaseModel)
		{
			if (broadphaseModel == "SweepAndPrune")     return BroadphaseType::SweepAndPrune;
			if (broadphaseModel == "MultiBoxPrune")     return BroadphaseType::MultiBoxPrune;
			if (broadphaseModel == "AutomaticBoxPrune") return BroadphaseType::AutomaticBoxPrune;

			VX_CORE_ASSERT(false, "Unknown Broadphase Type!");
			return BroadphaseType::SweepAndPrune;
		}

		static std::string FrictionTypeToString(FrictionType frictionModel)
		{
			switch (frictionModel)
			{
				case FrictionType::OneDirectional: return "OneDirectional";
				case FrictionType::Patch:          return "Patch";
				case FrictionType::TwoDirectional: return "TwoDirectional";
			}

			VX_CORE_ASSERT(false, "Unknown Friction Type!");
			return {};
		}

		static FrictionType FrictionTypeFromString(const std::string& frictionModel)
		{
			if (frictionModel == "OneDirectional") return FrictionType::OneDirectional;
			if (frictionModel == "Patch")          return FrictionType::Patch;
			if (frictionModel == "TwoDirectional") return FrictionType::TwoDirectional;

			VX_CORE_ASSERT(false, "Unknown Friction Type!");
			return FrictionType::OneDirectional;
		}

	}

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
				out << YAML::Key << "StartScene" << YAML::Value << props.General.StartScene.string();
			}
			out << YAML::EndMap; // General

			out << YAML::Key << "RendererProperties" << YAML::BeginMap; // RendererProperties
			{
				out << YAML::Key << "TriangleCullMode" << YAML::Value << props.RendererProps.TriangleCullMode;
				out << YAML::Key << "EnvironmentMapResolution" << YAML::Value << Renderer::GetEnvironmentMapResolution();
				out << YAML::Key << "PrefilterMapResolution" << YAML::Value << Renderer::GetPrefilterMapResolution();
				out << YAML::Key << "ShadowMapResolution" << YAML::Value << Renderer::GetShadowMapResolution();
				out << YAML::Key << "Exposure" << YAML::Value << Renderer::GetSceneExposure();
				out << YAML::Key << "Gamma" << YAML::Value << Renderer::GetSceneGamma();
				out << YAML::Key << "UseVSync" << YAML::Value << Application::Get().GetWindow().IsVSyncEnabled();
				out << YAML::Key << "DisplaySceneIconsInEditor" << YAML::Value << props.RendererProps.DisplaySceneIconsInEditor;
			}
			out << YAML::EndMap; // RendererProperties

			out << YAML::Key << "PhysicsProperties" << YAML::BeginMap; // Physics Properties
			{
				out << YAML::Key << "Physics3DColliderColor" << YAML::Value << props.PhysicsProps.Physics3DColliderColor;
				out << YAML::Key << "BroadphaseModel" << YAML::Value << Utils::BroadphaseTypeToString(props.PhysicsProps.BroadphaseModel);
				out << YAML::Key << "FrictionModel" << YAML::Value << Utils::FrictionTypeToString(props.PhysicsProps.FrictionModel);
				out << YAML::Key << "Physics2DColliderColor" << YAML::Value << props.PhysicsProps.Physics2DColliderColor;
				out << YAML::Key << "ShowPhysicsColliders" << YAML::Value << props.PhysicsProps.ShowColliders;
			}
			out << YAML::EndMap; // Physics Properties

			out << YAML::Key << "ScriptingProperties" << YAML::BeginMap; // Scripting Properties
			{
				out << YAML::Key << "ScriptBinaryPath" << YAML::Value << props.ScriptingProps.ScriptBinaryPath.string();
				out << YAML::Key << "EnableMonoDebugging" << YAML::Value << props.ScriptingProps.EnableMonoDebugging;
				out << YAML::Key << "ReloadAssemblyOnPlay" << YAML::Value << props.ScriptingProps.ReloadAssemblyOnPlay;
			}
			out << YAML::EndMap; // Scripting Properties

			out << YAML::Key << "EditorProperties" << YAML::BeginMap; // Editior Properties
			{
				out << YAML::Key << "FrameStepCount" << YAML::Value << props.EditorProps.FrameStepCount;
				out << YAML::Key << "EditorCameraFOV" << YAML::Value << props.EditorProps.EditorCameraFOV;
				out << YAML::Key << "DrawGridAxes" << YAML::Value << props.EditorProps.DrawEditorAxes;
				out << YAML::Key << "DrawGrid" << YAML::Value << props.EditorProps.DrawEditorGrid;
				out << YAML::Key << "MaximizeOnPlay" << YAML::Value << props.EditorProps.MaximizeOnPlay;
				out << YAML::Key << "MuteAudioSources" << YAML::Value << props.EditorProps.MuteAudioSources;
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
			props.General.Name = generalData["Name"].as<std::string>();
			props.General.StartScene = generalData["StartScene"].as<std::string>();
		}

		{
			auto rendererData = projectData["RendererProperties"];
			if (rendererData["TriangleCullMode"])
			{
				std::string triangleCullMode = rendererData["TriangleCullMode"].as<std::string>();
				if (!triangleCullMode.empty())
				{
					props.RendererProps.TriangleCullMode = triangleCullMode;
				}
			}
			props.RendererProps.EnvironmentMapResolution = rendererData["EnvironmentMapResolution"].as<float>();
			props.RendererProps.PrefilterMapResolution = rendererData["PrefilterMapResolution"].as<float>();
			props.RendererProps.ShadowMapResolution = rendererData["ShadowMapResolution"].as<float>();
			props.RendererProps.Exposure = rendererData["Exposure"].as<float>();
			props.RendererProps.Gamma = rendererData["Gamma"].as<float>();
			props.RendererProps.UseVSync = rendererData["UseVSync"].as<bool>();
			props.RendererProps.DisplaySceneIconsInEditor = rendererData["DisplaySceneIconsInEditor"].as<bool>();

			Renderer::SetProperties(props.RendererProps);
		}

		{
			auto physicsData = projectData["PhysicsProperties"];
			props.PhysicsProps.Physics3DColliderColor = physicsData["Physics3DColliderColor"].as<Math::vec4>();
			props.PhysicsProps.BroadphaseModel = Utils::BroadphaseTypeFromString(physicsData["BroadphaseModel"].as<std::string>());
			props.PhysicsProps.FrictionModel = Utils::FrictionTypeFromString(physicsData["FrictionModel"].as<std::string>());
			props.PhysicsProps.Physics2DColliderColor = physicsData["Physics2DColliderColor"].as<Math::vec4>();
			props.PhysicsProps.ShowColliders = physicsData["ShowPhysicsColliders"].as<bool>();
		}

		{
			auto scriptingData = projectData["ScriptingProperties"];
			props.ScriptingProps.ScriptBinaryPath = scriptingData["ScriptBinaryPath"].as<std::string>();
			props.ScriptingProps.EnableMonoDebugging = scriptingData["EnableMonoDebugging"].as<bool>();
			props.ScriptingProps.ReloadAssemblyOnPlay = scriptingData["ReloadAssemblyOnPlay"].as<bool>();
		}

		{
			auto editorData = projectData["EditorProperties"];
			props.EditorProps.FrameStepCount = editorData["FrameStepCount"].as<uint32_t>();
			props.EditorProps.EditorCameraFOV = editorData["EditorCameraFOV"].as<float>();
			props.EditorProps.DrawEditorAxes = editorData["DrawGridAxes"].as<bool>();
			props.EditorProps.DrawEditorGrid = editorData["DrawGrid"].as<bool>();
			props.EditorProps.MaximizeOnPlay = editorData["MaximizeOnPlay"].as<bool>();
			props.EditorProps.MuteAudioSources = editorData["MuteAudioSources"].as<bool>();
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
