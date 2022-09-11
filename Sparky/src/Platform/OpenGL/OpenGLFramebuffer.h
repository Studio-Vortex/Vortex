#pragma once

#include "Sparky/Renderer/Framebuffer.h"

namespace Sparky {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferProperties& props);
		~OpenGLFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void Resize(uint32_t width, uint32_t height) override;

		inline uint32_t GetColorAttachmentRendererID() const override { return m_ColorAttachment; }

		const FramebufferProperties& GetProperties() const override { return m_Properties; }

		void Invalidate();
	private:
		uint32_t m_RendererID = 0;
		uint32_t m_ColorAttachment = 0;
		uint32_t m_DepthAttachment = 0;
		FramebufferProperties m_Properties;
	};

}