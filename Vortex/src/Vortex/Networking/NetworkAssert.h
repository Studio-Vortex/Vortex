#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

#ifdef VX_ENABLE_ASSERTS
	#define VX_CHECK_NETWORK_RESULT(res, msg, ...) { if ((res) != 0) { VX_CORE_ASSERT(false, msg, __VA_ARGS__) } }
#else
	#define VX_CHECK_NETWORK_RESULT(res, msg, ...)
#endif // VX_ENABLE_ASSERTS

}
