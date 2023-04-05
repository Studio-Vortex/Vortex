#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Renderer/FramebufferTextureProperties.h"

#include <vector>

namespace Vortex {

	struct VORTEX_API FramebufferAttachmentProperties
	{
		FramebufferAttachmentProperties() = default;
		FramebufferAttachmentProperties(std::initializer_list<FramebufferTextureProperties> attachments)
			: Attachments(attachments) { }

		std::vector<FramebufferTextureProperties> Attachments;
	};

}
