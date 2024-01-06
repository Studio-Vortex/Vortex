#pragma once

#include "Vortex/Core/Base.h"

extern "C"
{
	typedef struct _MonoObject MonoObject;
}

namespace Vortex {

	struct VORTEX_API RT_ScriptInvokeResult
	{
		MonoObject* ReturnValue = nullptr;
		MonoObject* Exception = nullptr;

		RT_ScriptInvokeResult() = default;
	};

}
