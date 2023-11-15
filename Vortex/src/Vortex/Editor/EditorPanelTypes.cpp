#include "vxpch.h"
#include "EditorPanelTypes.h"

namespace Vortex {
	
	namespace Utils {
		
		std::string EditorPanelTypeToString(EditorPanelType type)
		{
			switch (type)
			{
				case EditorPanelType::SceneRenderer: return "Scene Renderer";
				case EditorPanelType::AssetRegistry: return "Asset Registry";
				case EditorPanelType::BuildSettings: return "Build Settings";
				case EditorPanelType::SystemManager: return "System Manager";
				case EditorPanelType::ShaderEditor:  return "Shader Editor";
				case EditorPanelType::Performance:   return "Performance";
				case EditorPanelType::SubModules:    return "Sub Modules";
				case EditorPanelType::ECSDebug:      return "ECS Registry View";
				case EditorPanelType::Console:       return "Console";
				case EditorPanelType::About:         return "About";
			}

			VX_CORE_ASSERT(false, "Unknown editor panel type");
			return "None";
		}

	}

}
