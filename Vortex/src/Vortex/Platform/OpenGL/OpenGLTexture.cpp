#include "vxpch.h"
#include "OpenGLTexture.h"

#include <Glad/glad.h>
#include <stb_image.h>
#include <stb_image_write.h>

namespace Vortex {
	
	namespace Utils {

		static int VortexImageWrapModeToGL(ImageWrap wrapMode)
		{
			switch (wrapMode)
			{
				case ImageWrap::Clamp:  return GL_CLAMP_TO_EDGE;
				case ImageWrap::Repeat: return GL_REPEAT;
			}

			VX_CORE_ASSERT(false, "Unknown Image Wrap Mode!");
			return 0;
		}

		static int VortexImageFilterModeToGL(ImageFilter filterMode)
		{
			switch (filterMode)
			{
				case ImageFilter::Linear:  return GL_LINEAR;
				case ImageFilter::Nearest: return GL_NEAREST;
			}

			VX_CORE_ASSERT(false, "Unknown Image Filter Mode!");
			return 0;
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureProperties& imageProps)
		: m_Properties(imageProps)
	{
		VX_PROFILE_FUNCTION();

		// slight hack for saving files
		if (m_Properties.Buffer || m_Properties.Channels > 0)
		{
			return;
		}

		// Create Texture from width & height
		if (m_Properties.Filepath.empty())
		{
			CreateImageFromWidthAndHeight();
			return;
		}

		// Create Texture from file
		stbi_set_flip_vertically_on_load(m_Properties.FlipVertical);

		const bool isHdrFile = m_Properties.TextureFormat == ImageFormat::RGBA16F || stbi_is_hdr(m_Properties.Filepath.c_str());
		
		if (isHdrFile)
		{
			CreateImageFromHDRFile();
		}
		else
		{
			CreateImageFromFile();
		}

		if (m_Properties.GenerateMipmaps)
		{
			glGenerateTextureMipmap(m_RendererID);
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
		VX_CORE_ASSERT(size == m_Properties.Width * m_Properties.Height * bytesPerPixel, "Data must be entire Texture!");
#endif // VX_ENABLE_ASSERTS

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Properties.Width, m_Properties.Height, m_DataFormat, GL_FLOAT, (const void*)data);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		VX_PROFILE_FUNCTION();

#ifdef VX_ENABLE_ASSERTS
		uint32_t bytesPerPixel = m_DataFormat == GL_RGBA ? 4 : 3;
		VX_CORE_ASSERT(size == m_Properties.Width * m_Properties.Height * bytesPerPixel, "Data must be entire Texture!");
#endif // VX_ENABLE_ASSERTS

		glTextureSubImage2D(m_RendererID, NULL, NULL, NULL, m_Properties.Width, m_Properties.Height, m_DataFormat, GL_UNSIGNED_BYTE, (const void*)data);
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

	void OpenGLTexture2D::SaveToFile() const
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(!m_Properties.Filepath.empty(), "Cannot write image to an empty filepath!");
		VX_CORE_ASSERT(m_Properties.Width != 0 && m_Properties.Height != 0, "Cannot write image with zero sized width or height!");

		stbi_flip_vertically_on_write(true);
		stbi_write_png(m_Properties.Filepath.c_str(), m_Properties.Width, m_Properties.Height, m_Properties.Channels, m_Properties.Buffer, m_Properties.Stride);
	}

	void OpenGLTexture2D::CreateImageFromWidthAndHeight()
	{
		const bool rgba32f = m_Properties.TextureFormat == ImageFormat::RGBA32F;

		m_InternalFormat = rgba32f ? GL_RGBA32F : GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Properties.Width, m_Properties.Height);

		int wrap = rgba32f ? GL_CLAMP_TO_EDGE : Utils::VortexImageWrapModeToGL(m_Properties.WrapMode);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);

		int filter = Utils::VortexImageFilterModeToGL(m_Properties.TextureFilter);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Properties.GenerateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : filter);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);

		m_Properties.IsLoaded = true;
	}

	void OpenGLTexture2D::CreateImageFromHDRFile()
	{
		int width, height, channels;

		float* dataF32 = nullptr;

		{
			VX_PROFILE_SCOPE("stbi_loadf - OpenGLTexture2D::OpenGLTexture2D(const std::string&, bool)");
			dataF32 = stbi_loadf(m_Properties.Filepath.c_str(), &width, &height, &channels, 0);
		}
		VX_CORE_ASSERT(dataF32, "Failed to load HDR Image!");

		m_Properties.Width = width;
		m_Properties.Height = height;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_Properties.Width, m_Properties.Height, 0, GL_RGB, GL_FLOAT, dataF32);

		int wrap = Utils::VortexImageWrapModeToGL(m_Properties.WrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

		int filter = Utils::VortexImageFilterModeToGL(m_Properties.TextureFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Properties.GenerateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

		stbi_image_free(dataF32);

		m_Properties.IsLoaded = true;
	}

	void OpenGLTexture2D::CreateImageFromFile()
	{
		int width, height, channels;

		stbi_uc* data = nullptr;

		{
			VX_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&, bool)");
			data = stbi_load(m_Properties.Filepath.c_str(), &width, &height, &channels, 0);
		}
		VX_CORE_ASSERT(data, "Failed to load Image!");

		if (data)
		{
			m_Properties.Width = width;
			m_Properties.Height = height;

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
			else if (channels == 1)
			{
				internalFormat = GL_R8;
				dataFormat = GL_RED;
			}

			m_InternalFormat = internalFormat;
			m_DataFormat = dataFormat;

			VX_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!");

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, internalFormat, m_Properties.Width, m_Properties.Height);

			int wrap = Utils::VortexImageWrapModeToGL(m_Properties.WrapMode);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrap);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrap);

			int filter = Utils::VortexImageFilterModeToGL(m_Properties.TextureFilter);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, m_Properties.GenerateMipmaps ? GL_LINEAR_MIPMAP_LINEAR : filter);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filter);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Properties.Width, m_Properties.Height, dataFormat, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);

			m_Properties.IsLoaded = true;
		}
	}

}
