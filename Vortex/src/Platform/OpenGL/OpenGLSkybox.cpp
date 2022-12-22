#include "vxpch.h"
#include "OpenGLSkybox.h"

#include "Vortex/Renderer/Texture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Vortex {

	OpenGLSkybox::OpenGLSkybox(const std::string& filepath)
		: m_Filepath(filepath)
	{
		SP_PROFILE_FUNCTION();

		LoadSkybox(filepath);
	}

	OpenGLSkybox::~OpenGLSkybox()
	{
		SP_PROFILE_FUNCTION();

		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);
	}

	inline void OpenGLSkybox::SetFilepath(const std::string& filepath)
	{
		m_Filepath = filepath;
		LoadSkybox(m_Filepath);
		m_PathChanged = true;
	}

	void OpenGLSkybox::Bind() const
	{
		SP_PROFILE_FUNCTION();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLSkybox::Unbind() const
	{
		SP_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void OpenGLSkybox::LoadEquirectangularMapFromPath(const std::string& path)
	{
		// Load HDR EquirectangularMap
		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

		if (!data)
		{
			VX_CORE_ASSERT(false, "Failed to load HDR Environment Map from path: {}", path.c_str());
			return;
		}

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);

		m_IsLoaded = true;
	}

	void OpenGLSkybox::LoadSkybox(const std::string& filepath)
	{
		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);

		if (std::filesystem::path(filepath).filename().extension() == ".hdr")
		{
			LoadEquirectangularMapFromPath(filepath);
			m_PathChanged = true;
			return;
		}
		else
		{
			VX_CORE_WARN("Cannot load HDR Environment Map, not a '.hdr' {}", filepath.c_str());
		}
	}

}
