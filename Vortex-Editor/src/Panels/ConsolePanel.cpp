#include "ConsolePanel.h"

#include <Vortex/Core/Log.h>

namespace Vortex {

	namespace Utils {

		const std::string& RemoveLogLevel(std::string& message)
		{
			size_t leftBracketPos = message.find_last_of('[');
			size_t rightBracketPos = message.find_last_of(']') + 1;

			if (leftBracketPos != std::string::npos && rightBracketPos != std::string::npos)
			{
				size_t logLevelLength = rightBracketPos - leftBracketPos;

				message.erase(leftBracketPos, logLevelLength);
				return message;
			}
			else
			{
				VX_CORE_ASSERT(false, "Failed to remove log level!");
				return "";
			}
		}

	}

	void ConsolePanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Console", &s_ShowPanel);

			// Retrieve the last 128 messages from the logger
			auto logMessages = Log::GetMessages(128);

			for (auto& message : logMessages)
			{
				ImVec4 red = { 0.9f, 0.1f, 0.1f, 1.0f };
				ImVec4 green = { 0.1f, 0.9f, 0.1f, 1.0f };
				ImVec4 yellow = { 0.9f, 0.9f, 0.1f, 1.0f };

				if (message.find("[trace]") != std::string::npos)
				{
					const std::string& traceMessage = Utils::RemoveLogLevel(message);
					Gui::Text(traceMessage.c_str());
				}

				if (message.find("[info]") != std::string::npos)
				{
					const std::string& infoMessage = Utils::RemoveLogLevel(message);
					Gui::TextColored(green, infoMessage.c_str());
				}

				if (message.find("[warning]") != std::string::npos)
				{
					const std::string& warningMessage = Utils::RemoveLogLevel(message);
					Gui::TextColored(yellow, warningMessage.c_str());
				}

				if (message.find("[error]") != std::string::npos)
				{
					const std::string& errorMessage = Utils::RemoveLogLevel(message);
					Gui::TextColored(red, errorMessage.c_str());
				}

				if (message.find("[critical]") != std::string::npos)
				{
					const std::string& criticalMessage = Utils::RemoveLogLevel(message);
					Gui::TextColored(red, criticalMessage.c_str());
				}
			}

			Gui::End();
		}
	}

}
