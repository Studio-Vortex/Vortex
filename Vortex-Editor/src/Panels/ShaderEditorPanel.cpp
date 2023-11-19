#include "ShaderEditorPanel.h"

namespace Vortex {

	void ShaderEditorPanel::OnEditorAttach()
	{
		m_TextEditor.SetShowWhitespaces(false);
	}

	void ShaderEditorPanel::OnEditorDetach() { }

	void ShaderEditorPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		std::string filepath;
		std::string srcCode;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		RenderShaderEditor();
			
		Gui::End();
	}

	void ShaderEditorPanel::RenderShaderEditor()
	{
		m_TextEditor.Render("Shader Editor", Gui::GetContentRegionAvail());

		// Accept a Shader from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filePath = std::filesystem::path(path);

				if (filePath.extension().string() == ".glsl")
					LoadShaderFile(std::filesystem::path(path));
				else
					VX_WARN("Could not load shader {}", filePath.filename().string());
			}
			Gui::EndDragDropTarget();
		}
	}

	void ShaderEditorPanel::LoadShaderFile(const std::filesystem::path& path)
	{
		std::string line;
		std::ifstream in(path, std::ios::in, std::ios::binary); // Read as binary input
		std::vector<std::string> lines;

		if (!in.is_open())
		{
			VX_CONSOLE_LOG_WARN("Could not load shader {}", path.filename().string());
		}

		in.seekg(0, std::ios::end); // move to the end of the file
		size_t size = in.tellg(); // get the size of the file

		in.seekg(0, std::ios::beg); // move to the beginning

		while (std::getline(in, line))
		{
			lines.push_back(line);
		}

		in.close(); // close the file

		m_CurrentShaderPath = path;

		m_TextEditor.SetTextLines(lines);
	}

}
