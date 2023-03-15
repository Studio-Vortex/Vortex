#pragma once

#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"

namespace Vortex {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() = default;
		OpenGLSkybox(const std::string& filepath);
		~OpenGLSkybox() override;

		void LoadFromFilepath(const std::string& filepath) override;
		const std::string& GetFilepath() const override;

		void Bind() const override;
		void Unbind() const override;

		inline bool ShouldReload() const override { return m_ShouldReload; }
		inline void SetShouldReload(bool reload) override { m_ShouldReload = reload; }

		uint32_t GetRendererID() const override;

		bool IsLoaded() const override;

	private:
		void LoadEquirectangularMapFromPath(const std::string& path);
		void LoadSkybox(const std::string& filepath);

	private:
		SharedReference<Texture2D> m_HDREnvironmentMap = nullptr;
		bool m_ShouldReload = false;
	};

}
