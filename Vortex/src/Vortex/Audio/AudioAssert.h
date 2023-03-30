#pragma once

#include <miniaudio/miniaudio.h>

namespace Vortex {

#ifdef VX_DEBUG
	#define VX_CHECK_AUDIO_RESULT(res, msg, ...) { if((res) != MA_SUCCESS) { VX_CORE_ASSERT(false, msg, __VA_ARGS__) } }
#else
	#define VX_CHECK_AUDIO_RESULT(res, msg, ...)
#endif // VX_DEBUG

}
