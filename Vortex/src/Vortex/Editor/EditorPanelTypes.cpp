#include "vxpch.h"
#include "EditorPanelTypes.h"

namespace Vortex {
	
	namespace Utils {
		
		std::string EditorPanelTypeToString(EditorPanelType type)
		{
			switch (type)
			{
				case EditorPanelType::SystemManager: return "System Manager";
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
