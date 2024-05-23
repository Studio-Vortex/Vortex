#include "vxpch.h"
#include "EditorPanelTypes.h"

namespace Vortex {
	
	namespace Utils {
		
		std::string EditorPanelTypeToString(EditorPanelType type)
		{
			switch (type)
			{
				case EditorPanelType::PhysicsMaterialEditor: return "Physics Material Editor";
				case EditorPanelType::ProjectSettings:       return "Project Settings";
				case EditorPanelType::NetworkManager:        return "Network Manager";
				case EditorPanelType::SceneHierarchy:        return "Scene Hierarchy";
				case EditorPanelType::PhysicsStats:          return "Physics Stats";
				case EditorPanelType::ContentBrowser:        return "Content Browser";
				case EditorPanelType::ScriptRegistry:        return "Script Registry";
				case EditorPanelType::MaterialEditor:        return "Material Editor";
				case EditorPanelType::SceneRenderer:         return "Scene Renderer";
				case EditorPanelType::AssetRegistry:         return "Asset Registry";
				case EditorPanelType::BuildSettings:         return "Build Settings";
				case EditorPanelType::SystemManager:         return "System Manager";
				case EditorPanelType::InputManager:          return "Input Manager";
				case EditorPanelType::ShaderEditor:          return "Shader Editor";
				case EditorPanelType::Performance:           return "Performance";
				case EditorPanelType::AudioMixer:            return "Audio Mixer";
				case EditorPanelType::SubModules:            return "Sub Modules";
				case EditorPanelType::ECSDebug:              return "ECS Registry View";
				case EditorPanelType::Console:               return "Console";
				case EditorPanelType::About:                 return "About";
			}

			VX_CORE_ASSERT(false, "Unknown editor panel type");
			return "None";
		}

	}

}
