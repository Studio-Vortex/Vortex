#pragma once

namespace Vortex {

	enum class VORTEX_API ImageFormat
	{
		None = 0,

		// Color
		RGBA8,
		RGBA16F,
		RGBA32F,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class VORTEX_API ImageWrap
	{
		None = 0,
		Clamp,
		Repeat,
	};

	enum class VORTEX_API ImageFilter
	{
		None = 0,
		Linear,
		Nearest,
	};

}
