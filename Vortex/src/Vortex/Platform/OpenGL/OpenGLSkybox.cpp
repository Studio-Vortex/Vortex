#include "vxpch.h"
#include "OpenGLSkybox.h"

#include "Vortex/Renderer/Texture.h"

#include <Glad/glad.h>
#include <stb_image.h>

namespace Vortex {

	OpenGLSkybox::OpenGLSkybox(const std::string& filepath)
	{
		LoadSkybox(filepath);
	}

	OpenGLSkybox::~OpenGLSkybox() { }

	void OpenGLSkybox::LoadFromFilepath(const std::string& filepath)
	{
		LoadSkybox(filepath);
	}

    const std::string& OpenGLSkybox::GetFilepath() const
    {
		if (m_HDREnvironmentMap)
			return m_HDREnvironmentMap->GetPath();

		return "";
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

	void OpenGLSkybox::Reload()
	{
		VX_PROFILE_FUNCTION();

		LoadSkybox(m_HDREnvironmentMap->GetPath());
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

	void OpenGLSkybox::LoadEquirectangularMapFromPath(const std::string& filepath)
	{
		VX_PROFILE_FUNCTION();

		ImageProperties hdrImageProps;
		hdrImageProps.Filepath = filepath;
		hdrImageProps.WrapMode = ImageWrap::Clamp;
		m_HDREnvironmentMap = Texture2D::Create(hdrImageProps);
	}

	void OpenGLSkybox::LoadSkybox(const std::string& filepath)
	{
		if (std::filesystem::path(filepath).filename().extension() == ".hdr")
		{
			LoadEquirectangularMapFromPath(filepath);
			m_PathChanged = true;
			return;
		}
		else
		{
			VX_CONSOLE_LOG_WARN("Cannot load HDR Environment Map, not a '.hdr' {}", filepath.c_str());
		}

		m_IsDirty = false;
	}

}
