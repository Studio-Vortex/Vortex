#include "vxpch.h"
#include "OpenGLSkybox.h"

#include "Vortex/Renderer/Texture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Vortex {

	OpenGLSkybox::OpenGLSkybox(const std::string& directoryPath)
		: m_Filepath(directoryPath)
	{
		SP_PROFILE_FUNCTION();

		LoadSkybox(directoryPath);
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

		if (!m_IsHDREquirectangularMap)
		{
			glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
			return;
		}

		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLSkybox::Unbind() const
	{
		SP_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
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

		// HDR Equirectangular map successfully loaded
		m_IsHDREquirectangularMap = true;
		m_IsLoaded = true;
	}

	void OpenGLSkybox::LoadSkyboxFromDirectory(const std::string& filepath)
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		int width, height, channels;
		{
			std::array<std::string, 6> facePaths;

			for (auto& directoryEntry : std::filesystem::directory_iterator(filepath))
			{
				const std::string& path = directoryEntry.path().string();

				if (path.find("right") != std::string::npos)
					facePaths[0] = path;
				if (path.find("left") != std::string::npos)
					facePaths[1] = path;
				if (path.find("top") != std::string::npos)
					facePaths[2] = path;
				if (path.find("bottom") != std::string::npos)
					facePaths[3] = path;
				if (path.find("front") != std::string::npos)
					facePaths[4] = path;
				if (path.find("back") != std::string::npos)
					facePaths[5] = path;
			}

			uint32_t i = 0;

			stbi_set_flip_vertically_on_load(false);

			for (auto& path : facePaths)
			{
				stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

				if (data)
				{
					glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
					stbi_image_free(data);
				}
				else
					VX_CORE_ASSERT(data, "Failed to load Image from: " + path);

				i++;
			}

			m_IsLoaded = true;
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	}

	void OpenGLSkybox::LoadSkybox(const std::string& filepath)
	{
		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);

		if (std::filesystem::is_directory(filepath))
		{
			LoadSkyboxFromDirectory(filepath);
			return;
		}

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
