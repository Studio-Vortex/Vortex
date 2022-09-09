#pragma once

#include "Sparky/Renderer/Texture.h"

typedef unsigned int GLenum;

namespace Sparky {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(uint32_t width, uint32_t height);
		OpenGLTexture2D(const std::string& path, bool flipVertical);
		~OpenGLTexture2D() override;

		inline uint32_t GetWidth() const { return m_Width; }
		inline uint32_t GetHeight() const { return m_Height; }

		void SetData(void* data, uint32_t size);

		void Bind(uint32_t slot) const override;
		void Unbind() const override;

	private:
		std::string m_Path;
		mutable uint32_t m_Slot;
		uint32_t m_Width;
		uint32_t m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};

}