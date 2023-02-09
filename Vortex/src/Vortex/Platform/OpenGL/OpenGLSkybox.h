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

		inline bool PathChanged() const override { return m_PathChanged; }
		inline void SetPathChanged(bool changed) override { m_PathChanged = changed; }

		inline bool IsDirty() const override { return m_IsDirty; }
		inline void SetIsDirty(bool dirty) override { m_IsDirty = dirty; }

		void Reload() override;

		uint32_t GetRendererID() const override;

		bool IsLoaded() const override;

	private:
		void LoadEquirectangularMapFromPath(const std::string& path);
		void LoadSkybox(const std::string& filepath);

	private:
		SharedRef<Texture2D> m_HDREnvironmentMap = nullptr;
		bool m_PathChanged = false;
		bool m_IsDirty = false;
	};

}
