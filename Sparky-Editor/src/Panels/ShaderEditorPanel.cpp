#include "ShaderEditorPanel.h"

#include <Sparky/Utils/PlatformUtils.h>

namespace Sparky {

	ShaderEditorPanel::ShaderEditorPanel()
	{
		m_TextEditor.SetShowWhitespaces(false);
		TextEditor::Breakpoints breakpoints;
		breakpoints.insert(1);
		m_TextEditor.SetBreakpoints(breakpoints);
	}

	void ShaderEditorPanel::OnGuiRender(bool showDefault)
	{
		if (s_ShowPanel || showDefault)
		{
			std::string filepath;
			std::string srcCode;

			Gui::Begin("Shader Editor", &s_ShowPanel);

			// Right (Code Editor)
			RenderShaderCodeEditor();
			
			Gui::End();
		}
	}

	void ShaderEditorPanel::RenderShaderCodeEditor()
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
					SP_WARN("Could not load shader {}", filePath.filename().string());
			}
			Gui::EndDragDropTarget();
		}
	}

	void ShaderEditorPanel::LoadShaderFile(const std::filesystem::path& path)
	{
		std::string line;
		std::ifstream in(path, std::ios::in, std::ios::binary); // Read as binary input
		std::vector<std::string> lines;

		if (in)
		{
			in.seekg(0, std::ios::end); // move to the end of the file
			size_t size = in.tellg(); // get the size of the file

			if (size != -1)
			{
				in.seekg(0, std::ios::beg); // move to the beginning
				
				while (std::getline(in, line))
					lines.push_back(line);

				in.close(); // close the file
			}
			else
				SP_WARN("Could not load shader {}", path.filename().string());
		}

		m_CurrentShaderPath = path;

		m_TextEditor.SetTextLines(lines);
	}

}
