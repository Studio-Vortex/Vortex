#pragma once

#include "Vortex/Renderer/Skybox.h"
#include "Vortex/Renderer/Texture.h"

namespace Vortex {

	class OpenGLSkybox : public Skybox
	{
	public:
		OpenGLSkybox() = default;
		OpenGLSkybox(const std::filesystem::path& filepath);
		~OpenGLSkybox() override;

		void LoadFromFilepath(const std::filesystem::path& filepath) override;
		const std::filesystem::path& GetFilepath() const override;

		SharedReference<Texture2D> GetEnvironmentMap() const override;

		void Bind() const override;
		void Unbind() const override;

		inline bool ShouldReload() const override { return m_ShouldReload; }
		inline void SetShouldReload(bool reload) override { m_ShouldReload = reload; }

		uint32_t GetRendererID() const override;

		bool IsLoaded() const override;

	private:
		void LoadEquirectangularMapFromPath(const std::filesystem::path& path);

	private:
		SharedReference<Texture2D> m_HDREnvironmentMap = nullptr;
		bool m_ShouldReload = false;
	};

}
