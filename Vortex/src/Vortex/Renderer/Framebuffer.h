#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	enum class VORTEX_API FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RGBA16F,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	struct VORTEX_API FramebufferTextureProperties
	{
		FramebufferTextureProperties() = default;
		FramebufferTextureProperties(FramebufferTextureFormat format)
			: TextureFormat(format) { }

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		// TODO: filtering/wrap
	};

	struct VORTEX_API FramebufferAttachmentProperties
	{
		FramebufferAttachmentProperties() = default;
		FramebufferAttachmentProperties(std::initializer_list<FramebufferTextureProperties> attachments)
			: Attachments(attachments) { }

		std::vector<FramebufferTextureProperties> Attachments;
	};

	struct VORTEX_API FramebufferProperties
	{
		uint32_t Width;
		uint32_t Height;
		FramebufferAttachmentProperties Attachments;
		uint32_t Samples;

		bool SwapChainTarget = false;
	};

	class VORTEX_API Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) const = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int clearValue) const = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FramebufferProperties& GetProperties() const = 0;

		static SharedRef<Framebuffer> Create(const FramebufferProperties& props);
	};

	class VORTEX_API HDRFramebuffer
	{
	public:
		virtual ~HDRFramebuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void BindEnvironmentCubemap() const = 0;
		virtual void BindIrradianceCubemap() const = 0;
		virtual void BindPrefilterCubemap() const = 0;
		virtual void BindBRDFLutTexture() const = 0;

		virtual void CreateIrradianceCubemap() = 0;
		virtual void CreatePrefilteredEnvironmentCubemap() = 0;
		virtual void CreateBRDFLutTexture() = 0;

		virtual void BindAndSetRenderbufferStorage(uint32_t width, uint32_t height) const = 0;

		virtual void SetEnvironmentCubemapFramebufferTexture(uint32_t index) const = 0;
		virtual void SetIrradianceCubemapFramebufferTexture(uint32_t index) const = 0;
		virtual void SetPrefilterCubemapFramebufferTexture(uint32_t index) const = 0;
		virtual void SetBRDFLutFramebufferTexture() const = 0;
		virtual void ClearColorAndDepthAttachments() const = 0;
		virtual void RescaleAndBindFramebuffer(uint32_t width, uint32_t height) const = 0;

		static SharedRef<HDRFramebuffer> Create(const FramebufferProperties& props);
	};

}
