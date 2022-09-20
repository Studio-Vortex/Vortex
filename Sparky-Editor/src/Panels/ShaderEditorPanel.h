#pragma once

#include <Sparky.h>

namespace Sparky {

	static constexpr size_t SHADER_BUFFER_SIZE = 5'000;

	class ShaderEditorPanel
	{
	public:
		ShaderEditorPanel() = default;

		void OnGuiRender(bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }
		void LoadShaderFile(const std::filesystem::path& path);

	private:
		char m_CodeBuffer[SHADER_BUFFER_SIZE] = "";
		bool m_ShaderLoaded = false;
		std::filesystem::path m_CurrentShaderPath = std::filesystem::path();
		inline static bool s_ShowPanel = false;
	};

}
