#include "ShaderEditorPanel.h"

#include <Sparky/Utils/PlatformUtils.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	void ShaderEditorPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			std::string filepath;
			std::string srcCode;

			Gui::Begin("Shader Editor", &s_ShowPanel);

			Gui::Columns(4);

			if (Gui::Button("Open Shader..."))
			{
				m_CurrentShaderPath = FileSystem::OpenFile("Shader File (*.glsl)\0*.glsl\0");
				SP_TRACE("{}", m_CurrentShaderPath);
				LoadShaderFile(m_CurrentShaderPath);
			}

			Gui::NextColumn();

			if (Gui::Button("Compile Shader"))
			{
				// TODO: Once we have an asset system we can recompile a specific shader on the fly
			}

			Gui::NextColumn();

			if (Gui::Button("Save"))
			{
				std::string filepath = FileSystem::SaveFile("Shader File (*.glsl)\0*.glsl\0");
				// TODO: Also need an asset system to save the shader at a specific path
			}

			Gui::NextColumn();

			if (Gui::Button("Clear TextBox"))
			{
				memset(m_CodeBuffer, 0, SHADER_BUFFER_SIZE);
			}

			Gui::Columns(1);

			Gui::BeginChild("Code Editor", Gui::GetWindowSize(), false);

			Gui::InputTextMultiline("##SrcCode", m_CodeBuffer, SHADER_BUFFER_SIZE, Gui::GetWindowSize());

			Gui::EndChild();

			// Accept a Shader from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path filePath = std::filesystem::path(path);

					if (filePath.extension().string() == ".glsl")
						LoadShaderFile(std::filesystem::path(g_AssetPath) / path);
					else
						SP_WARN("Could not load texture {}", filePath.filename().string());
				}
				Gui::EndDragDropTarget();
			}
			
			Gui::End();
		}
	}

	void ShaderEditorPanel::LoadShaderFile(const std::filesystem::path& path)
	{
		std::string result;
		std::ifstream in(path, std::ios::in, std::ios::binary); // Read as binary input

		if (in)
		{
			in.seekg(0, std::ios::end); // move to the end of the file
			size_t size = in.tellg(); // get the size of the file
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg); // move to the beginning
				in.read(&result[0], size); // copy the data
				in.close(); // close the file

				m_ShaderLoaded = true;
			}
			else
				SP_WARN("Could not load shader {}", path.filename().string());
		}

		for (size_t i = 0; i < result.size(); i++)
			m_CodeBuffer[i] = result[i];
	}

}