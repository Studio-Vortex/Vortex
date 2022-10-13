#include "ShaderEditorPanel.h"

#include <Sparky/Utils/PlatformUtils.h>

namespace Sparky {

	extern const std::filesystem::path g_AssetPath;

	void ShaderEditorPanel::OnGuiRender(bool showDefault)
	{
		if (s_ShowPanel || showDefault)
		{
			std::string filepath;
			std::string srcCode;

			Gui::Begin("Shader Editor", &s_ShowPanel);

			// Left
			static uint32_t selectedSetting = 0;
			Gui::BeginChild("Left Pane", ImVec2(150, 0), false);
			if (Gui::Button("Open Shader...", ImVec2{ Gui::GetContentRegionAvail().x, 0 }))
			{
				m_CurrentShaderPath = FileSystem::OpenFile("Shader File (*.glsl)\0*.glsl\0");
				SP_TRACE("{}", m_CurrentShaderPath);
				LoadShaderFile(m_CurrentShaderPath);
			}

			if (Gui::Button("Compile Shader", ImVec2{ Gui::GetContentRegionAvail().x, 0 }))
			{
				// TODO: Once we have an asset system we can recompile a specific shader on the fly
			}

			if (Gui::Button("Save", ImVec2{ Gui::GetContentRegionAvail().x, 0 }))
			{
				std::string filepath = FileSystem::SaveFile("Shader File (*.glsl)\0*.glsl\0");
				// TODO: Also need an asset system to save the shader at a specific path
			}

			if (Gui::Button("Clear TextBox", ImVec2{ Gui::GetContentRegionAvail().x, 0 }))
			{
				memset(m_CodeBuffer, 0, SHADER_BUFFER_SIZE);
				m_CurrentShaderPath = "";
			}
			Gui::EndChild();

			Gui::SameLine();

			// Right (Code Editor)
			Gui::BeginGroup();
			Gui::BeginChild("Right Pane", ImVec2(0, Gui::GetContentRegionAvail().y));
			if (Gui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
			{
				std::string shaderFileRelativePath;

				if (!m_CurrentShaderPath.empty())
				{
					size_t pos = m_CurrentShaderPath.string().find_last_of('\\');
					shaderFileRelativePath = m_CurrentShaderPath.string().substr(pos + 1, m_CurrentShaderPath.string().length());
				}

				if (Gui::BeginTabItem(m_CurrentShaderPath.empty() ? "Untitled.glsl" : shaderFileRelativePath.c_str()))
				{
					RenderShaderCodeEditor();
					Gui::EndTabItem();
				}
				Gui::EndTabBar();
			}
			Gui::EndChild();
			Gui::EndGroup();
			
			Gui::End();
		}
	}

	void ShaderEditorPanel::RenderShaderCodeEditor()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		Gui::InputTextMultiline("##SrcCode", m_CodeBuffer, SHADER_BUFFER_SIZE, Gui::GetContentRegionAvail());

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
					SP_WARN("Could not load shader {}", filePath.filename().string());
			}
			Gui::EndDragDropTarget();
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

		m_CurrentShaderPath = path;
	}

}
