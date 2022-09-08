#pragma once

#include "Sparky/Renderer/Texture.h"

namespace Sparky {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path, bool flipVertical);
		~OpenGLTexture2D() override;

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		void Bind(uint32_t slot) const override;

	private:
		std::string m_Path;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
	};

}