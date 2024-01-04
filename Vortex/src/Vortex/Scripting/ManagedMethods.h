#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	enum class VORTEX_API ManagedMethod
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
		OnGui,
	};

	namespace Utils {

		ManagedMethod ManagedMethodFromString(const std::string& str);
		std::string StringFromManagedMethod(ManagedMethod method);

	}

}
