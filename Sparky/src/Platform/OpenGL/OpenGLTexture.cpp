#include "sppch.h"
#include "OpenGLTexture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Sparky {
	
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height)
		: m_Width(width), m_Height(height), m_Slot()
	{
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool flipVertical)
		: m_Path(path), m_Slot()
	{
		if (flipVertical)
			stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		SP_CORE_ASSERT(data, "Failed to load Image!");

		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;

		if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}

		m_InternalFormat = internalFormat;
		m_DataFormat = dataFormat;

		SP_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
#ifdef SP_ENABLE_ASSERTS
		uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		SP_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire Texture!");
#endif // SP_ENABLE_ASSERTS

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, (const void*)data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		m_Slot = slot;
		glBindTextureUnit(slot, m_RendererID);
	}

    void OpenGLTexture2D::Unbind() const
    {
		glBindTextureUnit(m_Slot, m_RendererID);
    }

}