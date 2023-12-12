#pragma once

#include "Vortex/Renderer/Shader.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Utils/FileSystem.h"

#include <iostream>
#include <vector>

namespace Vortex {

	static void renderQuad();

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
		inline const std::vector<BloomMip>& MipChain() const { return m_MipChain; }

	private:
		uint32_t m_FramebufferRendererID = 0;
		std::vector<BloomMip> m_MipChain;

		bool m_Initialized = false;
	};

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

	class BloomRenderPass
	{
	public:
		BloomRenderPass() = default;
		~BloomRenderPass() = default;

		void InitRenderPass(const Math::vec2& viewportSize);
		void Destroy();

		void RenderPass(const Math::vec3& cameraPosition);

	private:
		SharedReference<Shader> m_BloomShader = nullptr;
		SharedReference<Shader> m_FinalCompositeShader = nullptr;
		BloomRenderer m_BloomRenderer;

		uint32_t m_HDRFramebufferRendererID = 0;
		uint32_t m_ColorBufferRendererIDs[2] = { 0 };

		bool m_Initialized = false;
	};

}
