#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Renderer/Texture.h"
#include "Vortex/Scene/Components.h"

namespace Vortex {

	// Forward declaration
	struct MSDFData;

	class VORTEX_API Font
	{
	public:
		Font(const std::filesystem::path& filepath);
		virtual ~Font();

		SharedRef<Texture2D> GetFontAtlas() const { return m_TextureAtlas; }
		const MSDFData* GetMSDFData() const { return m_MSDFData; }

		static void Init();
		static void Shutdown();

		static SharedRef<Font> GetDefaultFont();
		static SharedRef<Font> Create(const std::filesystem::path& filepath);

	private:
		inline static SharedRef<Font> s_DefaultFont = nullptr;

	private:
		std::filesystem::path m_Filepath;
		SharedRef<Texture2D> m_TextureAtlas = nullptr;
		MSDFData* m_MSDFData = nullptr;
	};

}
