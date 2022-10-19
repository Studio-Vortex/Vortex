#include "ConsolePanel.h"

#include <Sparky/Core/Log.h>

namespace Sparky {

	void ConsolePanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Console", &s_ShowPanel);

			auto logMessages = Log::GetMessages(128);

			if (!logMessages.empty())
			{
				for (auto& message : logMessages)
				{
					ImVec4 red = { 0.9f, 0.1f, 0.1f, 1.0f };
					ImVec4 green = { 0.1f, 0.9f, 0.1f, 1.0f };
					ImVec4 yellow = { 0.9f, 0.9f, 0.1f, 1.0f };

					auto RemoveLogLevelFromMessageFunc = [&]() {
						size_t leftBracketPos = message.find_last_of('[');
						size_t rightBracketPos = message.find_last_of(']');
						size_t logLevelLength = rightBracketPos - leftBracketPos;
						message.erase(leftBracketPos, logLevelLength + 1);
					};

					if (message.find("[trace]") != std::string::npos)
					{
						RemoveLogLevelFromMessageFunc();
						Gui::Text(message.c_str());
					}

					if (message.find("[info]") != std::string::npos)
					{
						RemoveLogLevelFromMessageFunc();
						Gui::TextColored(green, message.c_str());
					}

					if (message.find("[warning]") != std::string::npos)
					{
						RemoveLogLevelFromMessageFunc();
						Gui::TextColored(yellow, message.c_str());
					}

					if (message.find("[error]") != std::string::npos)
					{
						RemoveLogLevelFromMessageFunc();
						Gui::TextColored(red, message.c_str());
					}

					if (message.find("[critical]") != std::string::npos)
					{
						RemoveLogLevelFromMessageFunc();
						Gui::TextColored(red, message.c_str());
					}
				}
			}

			Gui::End();
		}
	}

}