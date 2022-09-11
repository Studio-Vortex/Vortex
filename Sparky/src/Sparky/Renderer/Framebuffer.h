#pragma once

#include "Sparky/Core/Base.h"

namespace Sparky {

	struct FramebufferProperties
	{
		uint32_t Width;
		uint32_t Height;
		// FramebufferFormat Format = 
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID() const = 0;

		virtual const FramebufferProperties& GetProperties() const = 0;

		static SharedRef<Framebuffer> Create(const FramebufferProperties& props);
	};

}
