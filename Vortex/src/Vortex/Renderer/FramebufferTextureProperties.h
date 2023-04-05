#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/Image.h"

namespace Vortex {

	struct VORTEX_API FramebufferTextureProperties
	{
		FramebufferTextureProperties() = default;
		FramebufferTextureProperties(ImageFormat format)
			: TextureFormat(format) { }

		ImageFormat TextureFormat = ImageFormat::None;
		// TODO: filtering/wrap
	};

}
