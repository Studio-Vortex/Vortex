#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	// Forward declaration
	struct MSDFData;

	class VORTEX_API Font : public Asset
	{
	public:
		Font() = default;
		Font(const std::filesystem::path& filepath);
		virtual ~Font() override;

		const std::filesystem::path& GetFontPath() const { return m_Filepath; }

		SharedReference<Texture2D> GetFontAtlas() const { return m_TextureAtlas; }
		const MSDFData* GetMSDFData() const { return m_MSDFData; }

		static void Init();
		static void Shutdown();

		ASSET_CLASS_TYPE(FontAsset)

		static SharedReference<Font> GetDefaultFont();
		static SharedReference<Font> Create(const std::filesystem::path& filepath);

	private:
		inline static SharedReference<Font> s_DefaultFont = nullptr;

	private:
		std::filesystem::path m_Filepath;
		SharedReference<Texture2D> m_TextureAtlas = nullptr;
		MSDFData* m_MSDFData = nullptr;
	};

}
