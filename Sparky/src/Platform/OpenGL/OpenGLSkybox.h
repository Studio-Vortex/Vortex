#pragma once

#include "Sparky/Renderer/Skybox.h"

namespace Sparky {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox(const std::string& directoryPath);
		~OpenGLSkybox() override;

		inline const std::string& GetDirectoryPath() const override { return m_DirectoryPath; }

		void Bind() const override;
		void Unbind() const override;

		inline bool IsLoaded() const override { return m_IsLoaded; }

	private:
		uint32_t m_RendererID = 0;
		std::string m_DirectoryPath;
		bool m_IsLoaded = false;
	};

}
