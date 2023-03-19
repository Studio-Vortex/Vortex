#include "vxpch.h"
#include "OpenGLSkybox.h"

#include "Vortex/Renderer/Texture.h"
#include "Vortex/Utils/FileSystem.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Vortex {

	OpenGLSkybox::OpenGLSkybox(const std::filesystem::path& filepath)
	{
		LoadEquirectangularMapFromPath(filepath);
	}

	OpenGLSkybox::~OpenGLSkybox() { }

	void OpenGLSkybox::LoadFromFilepath(const std::filesystem::path& filepath)
	{
		m_HDREnvironmentMap.Reset();
		LoadEquirectangularMapFromPath(filepath);
	}

    const std::filesystem::path& OpenGLSkybox::GetFilepath() const
    {
		if (m_HDREnvironmentMap)
			return m_HDREnvironmentMap->GetPath();

		return "";
    }

    SharedReference<Texture2D> OpenGLSkybox::GetEnvironmentMap() const
    {
		return m_HDREnvironmentMap;
    }

	void OpenGLSkybox::Bind() const
	{
		VX_PROFILE_FUNCTION();

		if (m_HDREnvironmentMap)
			m_HDREnvironmentMap->Bind();
	}

	void OpenGLSkybox::Unbind() const
	{
		VX_PROFILE_FUNCTION();

		if (m_HDREnvironmentMap)
			m_HDREnvironmentMap->Unbind();
	}

	uint32_t OpenGLSkybox::GetRendererID() const
	{
		if (m_HDREnvironmentMap)
			return m_HDREnvironmentMap->GetRendererID();

		return 0;
	}

	bool OpenGLSkybox::IsLoaded() const
	{
		if (m_HDREnvironmentMap)
			return m_HDREnvironmentMap->IsLoaded();

		return false;
	}

	void OpenGLSkybox::LoadEquirectangularMapFromPath(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		if (FileSystem::GetFileExtension(filepath) != ".hdr")
		{
			VX_CONSOLE_LOG_WARN("Cannot load HDR Environment Map, not a '.hdr' {}", filepath.string().c_str());
			return;
		}

		TextureProperties imageProps;
		imageProps.Filepath = filepath.string();
		imageProps.WrapMode = ImageWrap::Clamp;
		imageProps.TextureFormat = ImageFormat::RGBA16F;

		m_HDREnvironmentMap = Texture2D::Create(imageProps);

		m_ShouldReload = false;
	}

}
