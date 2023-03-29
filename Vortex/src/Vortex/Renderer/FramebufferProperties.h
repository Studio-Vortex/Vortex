#pragma once

#include "Vortex/Renderer/FramebufferAttachmentProperties.h"

#include <cstdint>

namespace Vortex {

	struct VORTEX_API FramebufferProperties
	{
		uint32_t Width = 0;
		uint32_t Height = 0;
		uint32_t Samples = 1;

		FramebufferAttachmentProperties Attachments;

		bool SwapChainTarget = false;
	};

}
