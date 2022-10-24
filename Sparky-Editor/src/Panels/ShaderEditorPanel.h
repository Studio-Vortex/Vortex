#pragma once

#include <Sparky.h>

namespace Sparky {

	static constexpr size_t SHADER_BUFFER_SIZE = 50'000;

	class ShaderEditorPanel
	{
	public:
		ShaderEditorPanel() = default;

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void LoadShaderFile(const std::filesystem::path& path);
		void RenderShaderCodeEditor();

	private:
		inline static bool s_ShowPanel = false;

	private:
		char m_CodeBuffer[SHADER_BUFFER_SIZE] = "";
		bool m_ShaderLoaded = false;
		std::filesystem::path m_CurrentShaderPath = std::filesystem::path();

	};

}
