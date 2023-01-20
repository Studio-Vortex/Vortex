#pragma once

#include "Vortex/Renderer/Framebuffer.h"

namespace Vortex {

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
			VX_CORE_ASSERT(index < m_ColorAttachments.size(), "Index out of bounds!");
			return m_ColorAttachments[index];
		}

		void BindColorTexture(uint32_t index) const override;

		inline const FramebufferProperties& GetProperties() const override { return m_Properties; }

		void Invalidate();

	private:
		uint32_t m_RendererID = 0;
		FramebufferProperties m_Properties;

		std::vector<FramebufferTextureProperties> m_ColorAttachmentProperties;
		FramebufferTextureProperties m_DepthAttachmentProperty;

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment = 0;
	};

	class OpenGLHDRFramebuffer : public HDRFramebuffer
	{
	public:
		OpenGLHDRFramebuffer(const FramebufferProperties& props);
		~OpenGLHDRFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void BindEnvironmentCubemap() const override;
		void BindIrradianceCubemap() const override;
		void BindIrradianceCubemap2() const override;
		void BindPrefilterCubemap() const override;

		void CreateEnvironmentCubemap(uint32_t environmentTexSize) override;
		void CreateIrradianceCubemap(uint32_t irradianceTexSize) override;
		void CreatePrefilteredEnvironmentCubemap(uint32_t prefilterTexSize) override;

		void BindAndGenerateEnvironmentMipMap() const override;
		void BindAndSetRenderbufferStorage(uint32_t width, uint32_t height) const override;

		void SetEnvironmentCubemapFramebufferTexture(uint32_t index) const override;
		void SetIrradianceCubemapFramebufferTexture(uint32_t index) const override;
		void SetPrefilterCubemapFramebufferTexture(uint32_t index, uint32_t mipLevel) const override;
		void ClearColorAndDepthAttachments() const override;
		void RescaleAndBindFramebuffer(uint32_t width, uint32_t height) const override;

	private:
		uint32_t m_BRDFLutTextureRendererID = 0;
		uint32_t m_PrefilteredEnvironmentCubemapRendererID = 0;
		uint32_t m_IrradianceCubemapRendererID = 0;
		uint32_t m_EnvironmentCubemapRendererID = 0;
		uint32_t m_CaptureFramebufferRendererID = 0;
		uint32_t m_CaptureRenderbufferRendererID = 0;
	};

	class OpenGLDepthMapFramebuffer : public DepthMapFramebuffer
	{
	public:
		OpenGLDepthMapFramebuffer(const FramebufferProperties& props);
		~OpenGLDepthMapFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void BindDepthTexture(uint32_t slot) const override;

		void ClearDepth(float value) const override;
		void ClearDepthAttachment() const override;

		inline uint32_t GetDepthTextureRendererID() const override { return m_DepthTextureRendererID; }

	private:
		uint32_t m_DepthMapFramebufferRendererID = 0;
		uint32_t m_DepthTextureRendererID = 0;
	};

	class OpenGLDepthCubeMapFramebuffer : public DepthCubemapFramebuffer
	{
	public:
		OpenGLDepthCubeMapFramebuffer(const FramebufferProperties& props);
		~OpenGLDepthCubeMapFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;

		void BindDepthTexture(uint32_t slot) const override;

		void ClearDepthAttachment() const override;

		inline uint32_t GetDepthCubemapTextureRendererID() const { return m_DepthCubemapTextureRendererID; }

	private:
		uint32_t m_DepthMapFramebufferRendererID = 0;
		uint32_t m_DepthCubemapTextureRendererID = 0;
	};

}
