#pragma once

#include "Vortex/Renderer/Skybox.h"

namespace Vortex {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() = default;
		OpenGLSkybox(const std::string& directoryPath);
		~OpenGLSkybox() override;

		inline void SetDirectoryPath(const std::string& directoryPath) override { m_DirectoryPath = directoryPath; LoadSkybox(m_DirectoryPath); }
		inline const std::string& GetDirectoryPath() const override { return m_DirectoryPath; }

		void Bind() const override;
		void Unbind() const override;

		inline bool IsLoaded() const override { return m_IsLoaded; }

	private:
		void LoadSkybox(const std::string& directoryPath);

	private:
		uint32_t m_RendererID = 0;
		std::string m_DirectoryPath;
		bool m_IsLoaded = false;
	};

}
