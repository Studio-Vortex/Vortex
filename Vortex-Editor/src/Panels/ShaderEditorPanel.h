#pragma once

#include <Vortex.h>

#include <TextEditor.h>

namespace Vortex {

	class ShaderEditorPanel
	{
	public:
		ShaderEditorPanel();

		void OnGuiRender();
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
