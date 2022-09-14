#pragma once

#include "Sparky/Core/Base.h"

namespace Sparky {

	enum class FrambufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	struct FramebufferTextureProperties
	{
		FramebufferTextureProperties() = default;
		FramebufferTextureProperties(FrambufferTextureFormat format)
			: TextureFormat(format) { }

		FrambufferTextureFormat TextureFormat = FrambufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct FramebufferAttachmentProperties
	{
		FramebufferAttachmentProperties() = default;
		FramebufferAttachmentProperties(std::initializer_list<FramebufferTextureProperties> attachments)
			: Attachments(attachments) { }

		std::vector<FramebufferTextureProperties> Attachments;
	};

	struct FramebufferProperties
	{
		uint32_t Width;
		uint32_t Height;
		FramebufferAttachmentProperties Attachments;
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

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FramebufferProperties& GetProperties() const = 0;

		static SharedRef<Framebuffer> Create(const FramebufferProperties& props);
	};

}
