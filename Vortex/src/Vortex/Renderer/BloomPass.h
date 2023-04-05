#pragma once

#include "Vortex/Core/Math/Math.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Shader.h"

#include "Vortex/Utils/FileSystem.h"

#include <Glad/glad.h>

#include <iostream>
#include <vector>

namespace Vortex {

	// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
	uint32_t quadVAO = 0;
	uint32_t quadVBO;
	void renderQuad()
	{
		if (quadVAO == 0)
		{
			float quadVertices[] = {
				// positions        // texture Coords
				-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
				 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
				 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
			};
			// setup plane VAO
			glGenVertexArrays(1, &quadVAO);
			glGenBuffers(1, &quadVBO);
			glBindVertexArray(quadVAO);
			glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);
	}

	// settings
	float bloomFilterRadius = 0.0025f;

	// bloom stuff
	struct BloomMip
	{
		Math::vec2 Size;
		Math::ivec2 IntSize;
		uint32_t TextureRendererID;
	};

	class BloomFramebuffer
	{
	public:
		BloomFramebuffer() = default;
		~BloomFramebuffer() = default;

		bool Init(uint32_t windowWidth, uint32_t windowHeight, uint32_t mipChainLength);
		void Destroy();
		void BindForWriting();
		const std::vector<BloomMip>& MipChain() const { return m_MipChain; }

	private:
		uint32_t m_FramebufferRendererID;
		std::vector<BloomMip> m_MipChain;

		bool m_Initialized = false;
	};

	bool BloomFramebuffer::Init(uint32_t windowWidth, uint32_t windowHeight, uint32_t mipChainLength)
	{
		if (m_Initialized)
			return true;

		glGenFramebuffers(1, &m_FramebufferRendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferRendererID);

		Math::vec2 mipSize((float)windowWidth, (float)windowHeight);
		Math::ivec2 mipIntSize((int)windowWidth, (int)windowHeight);

		for (uint32_t i = 0; i < mipChainLength; i++)
		{
			BloomMip mip;

			mipSize *= 0.5f;
			mipIntSize /= 2;
			mip.Size = mipSize;
			mip.IntSize = mipIntSize;

			glGenTextures(1, &mip.TextureRendererID);
			glBindTexture(GL_TEXTURE_2D, mip.TextureRendererID);
			// we are downscaling an HDR color buffer, so we need a float texture format
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F,
				(int)mipSize.x, (int)mipSize.y,
				0, GL_RGB, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			m_MipChain.emplace_back(mip);
		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, m_MipChain[0].TextureRendererID, 0);

		// setup attachments
		uint32_t attachments[1] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, attachments);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			VX_CORE_ASSERT(false, "Failed to create framebuffer!");
			return false;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		m_Initialized = true;
		return true;
	}

	void BloomFramebuffer::Destroy()
	{
		for (uint32_t i = 0; i < m_MipChain.size(); i++)
		{
			glDeleteTextures(1, &m_MipChain[i].TextureRendererID);
			m_MipChain[i].TextureRendererID = 0;
		}

		glDeleteFramebuffers(1, &m_FramebufferRendererID);
		m_FramebufferRendererID = 0;
		m_Initialized = false;
	}

	void BloomFramebuffer::BindForWriting()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferRendererID);
	}

	class BloomRenderer
	{
	public:
		BloomRenderer() = default;
		~BloomRenderer() = default;

		bool Init(uint32_t windowWidth, uint32_t windowHeight);
		void Destroy();
		void RenderBloomTexture(uint32_t srcTexture, float filterRadius);
		uint32_t BloomTexture();
		uint32_t BloomMip_i(uint32_t index);

	private:
		void RenderDownsamples(uint32_t srcTexture);
		void RenderUpsamples(float filterRadius);

	private:
		BloomFramebuffer m_Framebuffer;
		Math::ivec2 m_SrcViewportSize;
		Math::vec2 m_SrcViewportSizeFloat;

		SharedReference<Shader> m_DownsampleShader = nullptr;
		SharedReference<Shader> m_UpsampleShader = nullptr;

		bool m_Initalized = false;
		bool m_KarisAverageOnDownsample = true;
	};

	bool BloomRenderer::Init(uint32_t windowWidth, uint32_t windowHeight)
	{
		if (m_Initalized)
			return true;

		m_SrcViewportSize = Math::ivec2(windowWidth, windowHeight);
		m_SrcViewportSizeFloat = Math::vec2((float)windowWidth, (float)windowHeight);

		// Framebuffer
		const uint32_t num_bloom_mips = 6; // TODO: Play around with this value
		bool status = m_Framebuffer.Init(windowWidth, windowHeight, num_bloom_mips);

		// Shaders
		m_DownsampleShader = Renderer::GetShaderLibrary().Get("Bloom_Downsample");
		m_UpsampleShader = Renderer::GetShaderLibrary().Get("Bloom_Upsample");

		// Downsample
		m_DownsampleShader->Enable();
		m_DownsampleShader->SetInt("srcTexture", 0);
		glUseProgram(0);

		// Upsample
		m_UpsampleShader->Enable();
		m_UpsampleShader->SetInt("srcTexture", 0);
		glUseProgram(0);

		return true;
	}

	void BloomRenderer::Destroy()
	{
		m_Framebuffer.Destroy();
	}

	void BloomRenderer::RenderDownsamples(uint32_t srcTexture)
	{
		const std::vector<BloomMip>& mipChain = m_Framebuffer.MipChain();

		m_DownsampleShader->Enable();
		m_DownsampleShader->SetFloat2("srcResolution", m_SrcViewportSizeFloat);
		if (m_KarisAverageOnDownsample)
		{
			m_DownsampleShader->SetInt("mipLevel", 0);
		}

		// Bind srcTexture (HDR color buffer) as initial texture input
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, srcTexture);

		// Progressively downsample through the mip chain
		for (int i = 0; i < (int)mipChain.size(); i++)
		{
			const BloomMip& mip = mipChain[i];
			glViewport(0, 0, mip.Size.x, mip.Size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, mip.TextureRendererID, 0);

			// Render screen-filled quad of resolution of current mip
			renderQuad();

			// Set current mip resolution as srcResolution for next iteration
			m_DownsampleShader->SetFloat2("srcResolution", mip.Size);
			// Set current mip as texture input for next iteration
			glBindTexture(GL_TEXTURE_2D, mip.TextureRendererID);
			// Disable Karis average for consequent downsamples
			if (i == 0) { m_DownsampleShader->SetInt("mipLevel", 1); }
		}

		glUseProgram(0);
	}

	void BloomRenderer::RenderUpsamples(float filterRadius)
	{
		const std::vector<BloomMip>& mipChain = m_Framebuffer.MipChain();

		m_UpsampleShader->Enable();
		m_UpsampleShader->SetFloat("filterRadius", filterRadius);

		// Enable additive blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);

		for (uint32_t i = mipChain.size() - 1; i > 0; i--)
		{
			const BloomMip& mip = mipChain[i];
			const BloomMip& nextMip = mipChain[i - 1];

			// Bind viewport and texture from where to read
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, mip.TextureRendererID);

			// Set framebuffer render target (we write to this texture)
			glViewport(0, 0, nextMip.Size.x, nextMip.Size.y);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, nextMip.TextureRendererID, 0);

			// Render screen-filled quad of resolution of current mip
			renderQuad();
		}

		// Disable additive blending
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_BLEND);

		glUseProgram(0);
	}

	void BloomRenderer::RenderBloomTexture(uint32_t srcTexture, float filterRadius)
	{
		m_Framebuffer.BindForWriting();

		RenderDownsamples(srcTexture);
		RenderUpsamples(filterRadius);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Restore viewport
		glViewport(0, 0, m_SrcViewportSize.x, m_SrcViewportSize.y);
	}

	uint32_t BloomRenderer::BloomTexture()
	{
		return m_Framebuffer.MipChain()[0].TextureRendererID;
	}

	uint32_t BloomRenderer::BloomMip_i(uint32_t index)
	{
		const std::vector<BloomMip>& mipChain = m_Framebuffer.MipChain();
		uint32_t size = (uint32_t)mipChain.size();
		return mipChain[(index > size - 1) ? size - 1 : (index < 0) ? 0 : index].TextureRendererID;
	}

	class BloomRenderPass
	{
	public:
		BloomRenderPass() = default;
		~BloomRenderPass() = default;

		void InitRenderPass(const Math::vec2& viewportSize)
		{
			m_BloomShader = Renderer::GetShaderLibrary().Get("Bloom");
			m_FinalCompositeShader = Renderer::GetShaderLibrary().Get("Bloom_FinalComposite");

			// configure (floating point) framebuffers
			// ---------------------------------------
			glGenFramebuffers(1, &m_HDRFramebufferRendererID);
			glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFramebufferRendererID);

			// create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
			glGenTextures(2, m_ColorBufferRendererIDs);
			for (uint32_t i = 0; i < 2; i++)
			{
				glBindTexture(GL_TEXTURE_2D, m_ColorBufferRendererIDs[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewportSize.x, viewportSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				// attach texture to framebuffer
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_ColorBufferRendererIDs[i], 0);
			}
			// create and attach depth buffer (renderbuffer)
			uint32_t rboDepth;
			glGenRenderbuffers(1, &rboDepth);
			glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewportSize.x, viewportSize.y);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
			// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
			uint32_t attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, attachments);
			// finally check if framebuffer is complete
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "Framebuffer not complete!" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			// ping-pong-framebuffer for blurring
			uint32_t pingpongFBO[2];
			uint32_t pingpongColorbuffers[2];
			glGenFramebuffers(2, pingpongFBO);
			glGenTextures(2, pingpongColorbuffers);
			for (uint32_t i = 0; i < 2; i++)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
				glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, viewportSize.x, viewportSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				// we clamp to the edge as the blur filter would otherwise sample repeated texture values!
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);

				// also check if framebuffers are complete (no need for depth buffer)
				if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				{
					VX_CORE_ASSERT(false, "Failed to attach framebuffer texture!");
					continue;
				}
			}

			// shader configuration
			// --------------------
			m_BloomShader->Enable();
			m_BloomShader->SetInt("diffuseTexture", 0);
			m_FinalCompositeShader->Enable();
			m_FinalCompositeShader->SetInt("scene", 0);
			m_FinalCompositeShader->SetInt("bloomBlur", 1);

			// bloom renderer
			// --------------
			m_Initialized = m_BloomRenderer.Init(viewportSize.x, viewportSize.y);
		}

		void Destroy()
		{
			// cleanup
			m_BloomRenderer.Destroy();
			m_Initialized = false;
		}

		void RenderPass()
		{
			if (!m_Initialized)
				return;

			// render
					// ------
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// 1. render scene into floating point framebuffer
			// -----------------------------------------------
			glBindFramebuffer(GL_FRAMEBUFFER, m_HDRFramebufferRendererID);
			// set lighting uniforms

			// render scene

			// unbind render target
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			bool horizontal = true;

			m_BloomRenderer.RenderBloomTexture(m_ColorBufferRendererIDs[1], bloomFilterRadius);

			// 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
			// --------------------------------------------------------------------------------------------------------------------------
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_FinalCompositeShader->Enable();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_ColorBufferRendererIDs[0]);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_BloomRenderer.BloomTexture());

			m_FinalCompositeShader->SetFloat("exposure", Renderer::GetSceneExposure());
			renderQuad();
		}

	private:
		SharedReference<Shader> m_BloomShader = nullptr;
		SharedReference<Shader> m_FinalCompositeShader = nullptr;
		BloomRenderer m_BloomRenderer;

		uint32_t m_HDRFramebufferRendererID = 0;
		uint32_t m_ColorBufferRendererIDs[2] = { 0 };

		bool m_Initialized = false;
	};

}
