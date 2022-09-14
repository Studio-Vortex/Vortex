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
		int ReadPixel(uint32_t attachmentIndex, int x, int y) const override;

		void ClearAttachment(uint32_t attachmentIndex, int clearValue) const override;

		inline uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override
		{
			SP_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of bounds!");
			return m_ColorAttachments[index];
		}

		const FramebufferProperties& GetProperties() const override { return m_Properties; }

		void Invalidate();
	private:
		uint32_t m_RendererID = 0;
		FramebufferProperties m_Properties;

		std::vector<FramebufferTextureProperties> m_ColorAttachmentProperties;
		FramebufferTextureProperties m_DepthAttachmentProperty;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;
	};

}