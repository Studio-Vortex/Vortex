#include "vxpch.h"
#include "OpenGLTexture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Vortex {
	
	namespace Utils {

		static int VortexTextureWrapModeToGL(TextureWrap wrapMode)
		{
			switch (wrapMode)
			{
				case Vortex::TextureWrap::Clamp:  return GL_CLAMP_TO_EDGE;
				case Vortex::TextureWrap::Repeat: return GL_REPEAT;
			}

			VX_CORE_ASSERT(false, "Unknown Texture Wrap Mode!");
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, bool rgba32f)
		: m_Width(width), m_Height(height), m_Slot()
	{
		VX_PROFILE_FUNCTION();

		m_InternalFormat = rgba32f ? GL_RGBA32F : GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, rgba32f ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, rgba32f ? GL_CLAMP_TO_EDGE : GL_REPEAT);

		m_IsLoaded = true;
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, TextureWrap wrapMode, bool flipVertical)
		: m_Path(path), m_Slot()
	{
		VX_PROFILE_FUNCTION();

		if (flipVertical)
			stbi_set_flip_vertically_on_load(true);
		else
			stbi_set_flip_vertically_on_load(false);

		int width, height, channels;
		stbi_uc* data = nullptr;
		{
			VX_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&, bool)")
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		VX_CORE_ASSERT(data, "Failed to load Image!");

		if (data)
		{
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

			VX_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			int wrap = Utils::VortexTextureWrapModeToGL(wrapMode);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);

			m_IsLoaded = true;
		}
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		VX_PROFILE_FUNCTION();

		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::SetData(const void* data, uint32_t size)
	{
		VX_PROFILE_FUNCTION();

#ifdef VX_ENABLE_ASSERTS
		uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		VX_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire Texture!");
#endif // VX_ENABLE_ASSERTS

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Width, m_Height, m_DataFormat, GL_FLOAT, (const void*)data);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		VX_PROFILE_FUNCTION();

#ifdef VX_ENABLE_ASSERTS
		uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		VX_CORE_ASSERT(size == m_Width * m_Height * bytesPerPixel, "Data must be entire Texture!");
#endif // VX_ENABLE_ASSERTS

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, (const void*)data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		VX_PROFILE_FUNCTION();

		m_Slot = slot;
		glBindTextureUnit(slot, m_RendererID);
	}

    void OpenGLTexture2D::Unbind() const
    {
		VX_PROFILE_FUNCTION();

		glBindTextureUnit(m_Slot, 0);
    }

}
