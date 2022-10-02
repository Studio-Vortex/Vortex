#include "ConsolePanel.h"

#include <Sparky/Core/Log.h>

namespace Sparky {

	void ConsolePanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel)
		{
			Gui::Begin("Console", &s_ShowPanel);

			auto logMessages = Log::GetMessages();

			for (auto& message : logMessages)
			{
				ImVec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
				ImVec4 green = { 0.0f, 1.0f, 0.0f, 1.0f };
				ImVec4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };

				if (message.find("[trace]") != std::string::npos)
					Gui::Text(message.c_str());

				if (message.find("[info]") != std::string::npos)
					Gui::TextColored(green, message.c_str());

				if (message.find("[warning]") != std::string::npos)
					Gui::TextColored(yellow, message.c_str());

				if (message.find("[error]") != std::string::npos)
					Gui::TextColored(red, message.c_str());

				if (message.find("[critical]") != std::string::npos)
					Gui::TextColored(red, message.c_str());
			}

			Gui::End();
		}
	}

}