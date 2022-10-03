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

			auto logMessages = Log::GetMessages(128);

			if (!logMessages.empty())
			{
				for (auto& message : logMessages)
				{
					ImVec4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
					ImVec4 green = { 0.0f, 1.0f, 0.0f, 1.0f };
					ImVec4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };

					auto removeLogLevelFromMessage = [&]() {
						size_t leftBracketPos = message.find_last_of('[');
						size_t rightBracketPos = message.find_last_of(']');
						size_t logLevelLength = rightBracketPos - leftBracketPos;
						message.erase(leftBracketPos, logLevelLength + 1);
					};

					if (message.find("[trace]") != std::string::npos)
					{
						removeLogLevelFromMessage();
						Gui::Text(message.c_str());
					}

					if (message.find("[info]") != std::string::npos)
					{
						removeLogLevelFromMessage();
						Gui::TextColored(green, message.c_str());
					}

					if (message.find("[warning]") != std::string::npos)
					{
						removeLogLevelFromMessage();
						Gui::TextColored(yellow, message.c_str());
					}

					if (message.find("[error]") != std::string::npos)
					{
						removeLogLevelFromMessage();
						Gui::TextColored(red, message.c_str());
					}

					if (message.find("[critical]") != std::string::npos)
					{
						removeLogLevelFromMessage();
						Gui::TextColored(red, message.c_str());
					}
				}
			}

			Gui::End();
		}
	}

}