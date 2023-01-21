#include "vxpch.h"
#include "OpenGLFramebuffer.h"

#include "Vortex/Renderer/Renderer.h"

#include <Glad/glad.h>

namespace Vortex {

	static constexpr uint32_t MAX_FRAME_BUFFER_SIZE = 8'192;

	namespace Utils
	{
		static GLenum TextureTarget(bool multisampled)
		{
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled), count, outID);
		}

		static void BindTexture(bool multisampled, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index)
		{
			bool multisampled = samples > 1;
			
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);

				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height)
		{
			bool multisampled = samples > 1;

			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);

				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8: return true;
			}

			return false;
		}

		static GLenum VortexFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
				case FramebufferTextureFormat::RGBA8:       return GL_RGBA8;
				case FramebufferTextureFormat::RGBA16F:     return GL_RGBA16F;
				case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}

			VX_CORE_ASSERT(false, "Unknown texture format!");
			return 0;
		}
	}

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferProperties& props)
		: m_Properties(props)
	{
		for (auto& format : m_Properties.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentProperties.emplace_back(format);
			else
				m_DepthAttachmentProperty = format;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		if (m_RendererID)
			glDeleteFramebuffers(1, &m_RendererID);

		if (!m_ColorAttachments.empty())
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		
		if (m_DepthAttachment)
			glDeleteTextures(1, &m_DepthAttachment);
	}

	void OpenGLFramebuffer::BindColorTexture(uint32_t index) const
	{
		Utils::BindTexture(m_Properties.Samples > 1, m_ColorAttachments[index]);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Properties.Samples > 1;

		// Attachments
		if (m_ColorAttachmentProperties.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentProperties.size());
			Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multisample, m_ColorAttachments[i]);
				switch (m_ColorAttachmentProperties[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Properties.Samples, GL_RGBA8, GL_RGBA, m_Properties.Width, m_Properties.Height, i);
						break;
					case FramebufferTextureFormat::RGBA16F:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Properties.Samples, GL_RGBA16F, GL_RGBA, m_Properties.Width, m_Properties.Height, i);
						break;
					case FramebufferTextureFormat::RED_INTEGER:
						Utils::AttachColorTexture(m_ColorAttachments[i], m_Properties.Samples, GL_R32I, GL_RED_INTEGER, m_Properties.Width, m_Properties.Height, i);
						break;
				}
			}
		}

		if (m_DepthAttachmentProperty.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, m_DepthAttachment);

			switch (m_DepthAttachmentProperty.TextureFormat)
			{
				case FramebufferTextureFormat::DEPTH24STENCIL8:
					Utils::AttachDepthTexture(m_DepthAttachment, m_Properties.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Properties.Width, m_Properties.Height);
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			VX_CORE_ASSERT(m_ColorAttachments.size() <= 4, "Too many Color Attacments");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		VX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to complete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > MAX_FRAME_BUFFER_SIZE || height > MAX_FRAME_BUFFER_SIZE)
		{
			VX_CORE_WARN("Attempted to resize Framebuffer to {}, {}", width, height);
			return;
		}

		// Set the framebuffers size to the new size
		m_Properties.Width = width;
		m_Properties.Height = height;

		// Destroy and recreate the framebuffer because the size has changed
		Invalidate();
	}

	void OpenGLFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(NULL, NULL, m_Properties.Width, m_Properties.Height);
	}

	void OpenGLFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, NULL);
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) const
	{
		VX_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Index out of bounds!");

		int pixelData;

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

		return pixelData;
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int clearValue) const
	{
		VX_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(), "Index out of bounds!");

		auto& props = m_ColorAttachmentProperties[attachmentIndex];

		glClearTexImage(m_ColorAttachments[attachmentIndex], 0,
			Utils::VortexFBTextureFormatToGL(props.TextureFormat), GL_INT, &clearValue);
	}

	void OpenGLFramebuffer::ClearDepthAttachment() const
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	OpenGLHDRFramebuffer::OpenGLHDRFramebuffer(const FramebufferProperties& props)
	{
		glGenFramebuffers(1, &m_CaptureFramebufferRendererID);
		glGenRenderbuffers(1, &m_CaptureRenderbufferRendererID);

		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFramebufferRendererID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRenderbufferRendererID);

		float environmentMapResolution = Renderer::GetEnvironmentMapResolution();

		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, environmentMapResolution, environmentMapResolution);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_CaptureRenderbufferRendererID);

		VX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to complete!");
	}

	OpenGLHDRFramebuffer::~OpenGLHDRFramebuffer()
	{
		if (m_EnvironmentCubemapRendererID)
			glDeleteTextures(1, &m_EnvironmentCubemapRendererID);

		if (m_IrradianceCubemapRendererID)
			glDeleteTextures(1, &m_IrradianceCubemapRendererID);

		if (m_PrefilteredEnvironmentCubemapRendererID)
			glDeleteTextures(1, &m_PrefilteredEnvironmentCubemapRendererID);

		if (m_BRDFLutTextureRendererID)
			glDeleteTextures(1, &m_BRDFLutTextureRendererID);

		if (m_CaptureRenderbufferRendererID)
			glDeleteRenderbuffers(1, &m_CaptureRenderbufferRendererID);

		if (m_CaptureFramebufferRendererID)
			glDeleteFramebuffers(1, &m_CaptureFramebufferRendererID);
	}

	void OpenGLHDRFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFramebufferRendererID);
	}

	void OpenGLHDRFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLHDRFramebuffer::BindEnvironmentCubemap() const
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemapRendererID);
	}

	void OpenGLHDRFramebuffer::BindIrradianceCubemap() const
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceCubemapRendererID);
	}

	void OpenGLHDRFramebuffer::BindPrefilterCubemap() const
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilteredEnvironmentCubemapRendererID);
	}

	void OpenGLHDRFramebuffer::CreateEnvironmentCubemap(uint32_t environmentTexSize)
	{
		glGenTextures(1, &m_EnvironmentCubemapRendererID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemapRendererID);

		float environmentMapResoulution = Renderer::GetEnvironmentMapResolution();

		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB16F,
				environmentTexSize,
				environmentTexSize,
				0,
				GL_RGB,
				GL_FLOAT,
				nullptr
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void OpenGLHDRFramebuffer::BindAndGenerateEnvironmentMipMap() const
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_EnvironmentCubemapRendererID);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	void OpenGLHDRFramebuffer::CreateIrradianceCubemap(uint32_t irradianceTexSize)
	{
		glGenTextures(1, &m_IrradianceCubemapRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_IrradianceCubemapRendererID);
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB16F,
				irradianceTexSize,
				irradianceTexSize,
				0,
				GL_RGB,
				GL_FLOAT,
				nullptr
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	void OpenGLHDRFramebuffer::CreatePrefilteredEnvironmentCubemap(uint32_t prefilterTexSize)
	{
		// If there are a large number of smooth materials you may want to increase the resolution here
		glGenTextures(1, &m_PrefilteredEnvironmentCubemapRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_PrefilteredEnvironmentCubemapRendererID);
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB16F,
				prefilterTexSize,
				prefilterTexSize,
				0,
				GL_RGB,
				GL_FLOAT,
				nullptr
			);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	void OpenGLHDRFramebuffer::BindAndSetRenderbufferStorage(uint32_t width, uint32_t height) const
	{
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRenderbufferRendererID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	}

	void OpenGLHDRFramebuffer::SetEnvironmentCubemapFramebufferTexture(uint32_t index) const
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, m_EnvironmentCubemapRendererID, 0);
	}

	void OpenGLHDRFramebuffer::SetIrradianceCubemapFramebufferTexture(uint32_t index) const
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, m_IrradianceCubemapRendererID, 0);
	}

	void OpenGLHDRFramebuffer::SetPrefilterCubemapFramebufferTexture(uint32_t index, uint32_t mipLevel) const
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, m_PrefilteredEnvironmentCubemapRendererID, mipLevel);
	}

	void OpenGLHDRFramebuffer::ClearColorAndDepthAttachments() const
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLHDRFramebuffer::RescaleAndBindFramebuffer(uint32_t width, uint32_t height) const
	{
		// Rescale Capture Framebuffer to the new resolution
			// we can store the map at a low resolution (32x32) and let
			// opengl's linear filtering do most of the work
		glBindFramebuffer(GL_FRAMEBUFFER, m_CaptureFramebufferRendererID);
		glBindRenderbuffer(GL_RENDERBUFFER, m_CaptureRenderbufferRendererID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	}

	OpenGLDepthMapFramebuffer::OpenGLDepthMapFramebuffer(const FramebufferProperties& props)
	{
		glGenFramebuffers(1, &m_DepthMapFramebufferRendererID);

		// Create Depth Texture
		glGenTextures(1, &m_DepthTextureRendererID);
		glBindTexture(GL_TEXTURE_2D, m_DepthTextureRendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, props.Width, props.Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		// Attach depth texture to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebufferRendererID);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTextureRendererID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		VX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to complete!");
	}

	OpenGLDepthMapFramebuffer::~OpenGLDepthMapFramebuffer()
	{
		if (m_DepthTextureRendererID)
			glDeleteTextures(1, &m_DepthTextureRendererID);

		if (m_DepthMapFramebufferRendererID)
			glDeleteFramebuffers(1, &m_DepthMapFramebufferRendererID);
	}

	void OpenGLDepthMapFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_DepthMapFramebufferRendererID);
	}

	void OpenGLDepthMapFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}

	void OpenGLDepthMapFramebuffer::BindDepthTexture(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_DepthTextureRendererID);
	}

	void OpenGLDepthMapFramebuffer::ClearDepth(float value) const
	{
		glClearDepth(static_cast<double>(value));
	}

	void OpenGLDepthMapFramebuffer::ClearDepthAttachment() const
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	OpenGLDepthCubeMapFramebuffer::OpenGLDepthCubeMapFramebuffer(const FramebufferProperties& props)
	{
		glGenFramebuffers(1, &m_DepthMapFramebufferRendererID);

		// Create Depth Cubemap Texture
		glGenTextures(1, &m_DepthCubemapTextureRendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemapTextureRendererID);
		for (uint32_t i = 0; i < 6; i++)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, props.Width, props.Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// Attach to framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebufferRendererID);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_DepthCubemapTextureRendererID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		VX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to complete!");
	}

	OpenGLDepthCubeMapFramebuffer::~OpenGLDepthCubeMapFramebuffer()
	{
		if (m_DepthCubemapTextureRendererID)
			glDeleteTextures(1, &m_DepthCubemapTextureRendererID);

		if (m_DepthMapFramebufferRendererID)
			glDeleteFramebuffers(1, &m_DepthMapFramebufferRendererID);
	}

	void OpenGLDepthCubeMapFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFramebufferRendererID);
	}

	void OpenGLDepthCubeMapFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLDepthCubeMapFramebuffer::BindDepthTexture(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_DepthCubemapTextureRendererID);
	}

	void OpenGLDepthCubeMapFramebuffer::ClearDepthAttachment() const
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	OpenGLGaussianBlurFramebuffer::OpenGLGaussianBlurFramebuffer(const FramebufferProperties& props)
	{
		glGenFramebuffers(2, m_BlurFramebufferRendererIDs);
		glGenTextures(2, m_BlurTextures);

		for (uint32_t i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFramebufferRendererIDs[i]);
			glBindTexture(GL_TEXTURE_2D, m_BlurTextures[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, props.Width, props.Height, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BlurTextures[i], 0);

			VX_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer failed to complete!");
		}
	}

	OpenGLGaussianBlurFramebuffer::~OpenGLGaussianBlurFramebuffer()
	{
		glDeleteTextures(2, m_BlurTextures);
		glDeleteFramebuffers(2, m_BlurFramebufferRendererIDs);
	}

	void OpenGLGaussianBlurFramebuffer::Bind(uint32_t horizontal) const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_BlurFramebufferRendererIDs[horizontal]);
	}

	void OpenGLGaussianBlurFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLGaussianBlurFramebuffer::BindColorTexture(uint32_t horizontal) const
	{
		glBindTexture(GL_TEXTURE_2D, m_BlurTextures[horizontal]);
	}

}
