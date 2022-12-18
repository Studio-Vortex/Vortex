#pragma once

#include "Vortex/Renderer/Skybox.h"

namespace Vortex {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() = default;
		OpenGLSkybox(const std::string& filepath);
		~OpenGLSkybox() override;

		inline void SetFilepath(const std::string& filepath) override;
		inline const std::string& GetFilepath() const override { return m_Filepath; }

		void Bind() const override;
		void Unbind() const override;

		bool PathChanged() const override { return m_PathChanged; }
		void SetPathChanged(bool changed) override { m_PathChanged = changed; }

		uint32_t GetRendererID() const override { return m_RendererID; }

		inline bool IsHDREquirectangularMap() const override { return m_IsHDREquirectangularMap; }
		inline bool IsLoaded() const override { return m_IsLoaded; }

	private:
		void LoadEquirectangularMapFromPath(const std::string& path);
		void LoadSkyboxFromDirectory(const std::string& filepath);
		void LoadSkybox(const std::string& filepath);

	private:
		uint32_t m_RendererID = 0;
		std::string m_Filepath;
		bool m_IsHDREquirectangularMap = false;
		bool m_IsLoaded = false;
		bool m_PathChanged = false;
	};

}
