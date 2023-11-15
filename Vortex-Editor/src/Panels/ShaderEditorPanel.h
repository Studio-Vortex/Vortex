#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

#include <TextEditor.h>

namespace Vortex {

	class ShaderEditorPanel : public EditorPanel
	{
	public:
		~ShaderEditorPanel() override = default;

		void OnEditorAttach() override;
		void OnEditorDetach() override;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(ShaderEditor)

	private:
		void LoadShaderFile(const std::filesystem::path& path);
		void RenderShaderEditor();

	private:
		std::filesystem::path m_CurrentShaderPath = std::filesystem::path();
		TextEditor m_TextEditor;
	};

}
