#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {
	
	enum class VORTEX_API ScriptMethod
	{
		// Initialization
		OnAwake,
		OnEnable,
		OnCreate,

		// Game-Logic
		OnUpdate,
		OnPostUpdate,

		// Physics
		OnCollisionEnter,
		OnCollisionExit,
		OnTriggerEnter,
		OnTriggerExit,
		OnFixedJointDisconnected,

		// Decommissioning
		OnDisable,
		OnDestroy,
		
		// Rendering
		OnDebugRender,

		// Gui
		OnGuiRender,
	};

	namespace Utils {

		ScriptMethod ScriptMethodFromString(const std::string& str);
		std::string StringFromScriptMethod(ScriptMethod method);

	}

}
