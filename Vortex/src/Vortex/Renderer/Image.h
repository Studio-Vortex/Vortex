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

	struct ImageProperties
	{
		std::string Filepath = "";
		uint32_t Width = 0;
		uint32_t Height = 0;
		ImageWrap WrapMode = ImageWrap::Repeat;
		ImageFormat TextureFormat = ImageFormat::RGBA8;
		bool FlipVertical = true;
		bool IsLoaded = false;

		// Only used for writing to file
		uint32_t Channels = 0;
		const void* Buffer = nullptr;
		uint32_t Stride = 0;
	};

}
