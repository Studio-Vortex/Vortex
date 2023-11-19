#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {
	
	enum class VORTEX_API EditorPanelType
	{
		None = 0,
		PhysicsMaterialEditor,
		ProjectSettings,
		NetworkManager,
		SceneHierarchy,
		PhysicsStats,
		ContentBrowser,
		ScriptRegistry,
		MaterialEditor,
		SceneRenderer,
		AssetRegistry,
		BuildSettings,
		SystemManager,
		ShaderEditor,
		Performance,
		SubModules,
		ECSDebug,
		Console,
		About,
	};

#define EDITOR_PANEL_TYPE(type) static EditorPanelType GetStaticType() { return EditorPanelType::##type; }\
								virtual EditorPanelType GetPanelType() const override { return GetStaticType(); }

	namespace Utils {

		std::string EditorPanelTypeToString(EditorPanelType type);

	}

}
