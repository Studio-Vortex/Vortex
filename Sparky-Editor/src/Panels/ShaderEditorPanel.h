#pragma once

#include <Sparky.h>

#include <TextEditor.h>

namespace Sparky {

	class ShaderEditorPanel
	{
	public:
		ShaderEditorPanel();

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void LoadShaderFile(const std::filesystem::path& path);
		void RenderShaderCodeEditor();

	private:
		inline static bool s_ShowPanel = false;

	private:
		std::filesystem::path m_CurrentShaderPath = std::filesystem::path();
		TextEditor m_TextEditor;
	};

}
