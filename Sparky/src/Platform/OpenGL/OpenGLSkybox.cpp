#include "sppch.h"
#include "OpenGLSkybox.h"

#include "Sparky/Renderer/Texture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Sparky {

	OpenGLSkybox::OpenGLSkybox(const std::string& directoryPath)
		: m_DirectoryPath(directoryPath)
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

	void OpenGLSkybox::Bind() const
	{
		SP_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
	}

	void OpenGLSkybox::Unbind() const
	{
		SP_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void OpenGLSkybox::LoadSkybox(const std::string& directoryPath)
	{
		if (m_RendererID)
			glDeleteTextures(1, &m_RendererID);

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

		int width, height, channels;
		{
			std::array<std::string, 6> facePaths;

			for (auto& directoryEntry : std::filesystem::directory_iterator(directoryPath))
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
					SP_CORE_ASSERT(data, "Failed to load Image from: " + path);

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

}
