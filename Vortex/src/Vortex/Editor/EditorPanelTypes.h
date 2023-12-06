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
		AudioMixer,
		SubModules,
		ECSDebug,
		Console,
		About,
	};

	namespace Utils {

		std::string EditorPanelTypeToString(EditorPanelType type);

	}

}
