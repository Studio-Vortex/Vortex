#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	enum class VORTEX_API ScriptMethod
	{
		OnAwake,
		OnCreate,
		OnUpdateDelta,
		OnUpdate,
		OnDestroy,
		OnCollisionEnter,
		OnCollisionExit,
		OnTriggerEnter,
		OnTriggerExit,
		OnFixedJointDisconnected,
		OnEnable,
		OnDisable,
		OnDebugRender,
		OnGuiRender,
	};

	namespace Utils {

		ScriptMethod ScriptMethodFromString(const std::string& str);
		std::string StringFromScriptMethod(ScriptMethod method);

	}

}
