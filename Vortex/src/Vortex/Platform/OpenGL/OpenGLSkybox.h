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
		inline const std::string& GetFilepath() const override { return m_HDREnvironmentMap->GetPath(); }

		void Bind() const override;
		void Unbind() const override;

		bool PathChanged() const override { return m_PathChanged; }
		void SetPathChanged(bool changed) override { m_PathChanged = changed; }

		bool IsDirty() const override { return m_IsDirty; }
		void SetIsDirty(bool dirty) override { m_IsDirty = dirty; }

		void Reload() override;

		inline uint32_t GetRendererID() const override { return m_HDREnvironmentMap->GetRendererID(); }

		inline bool IsLoaded() const override { return m_HDREnvironmentMap->IsLoaded(); }

	private:
		void LoadEquirectangularMapFromPath(const std::string& path);
		void LoadSkybox(const std::string& filepath);

	private:
		SharedRef<Texture2D> m_HDREnvironmentMap = nullptr;
		bool m_PathChanged = false;
		bool m_IsDirty = false;
	};

}
