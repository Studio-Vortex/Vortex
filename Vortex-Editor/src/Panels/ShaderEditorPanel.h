#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

#include <TextEditor.h>

namespace Vortex {

	class ShaderEditorPanel : public EditorPanel
	{
	public:
		ShaderEditorPanel();
		~ShaderEditorPanel() override = default;

		void OnGuiRender() override;
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override {}
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
