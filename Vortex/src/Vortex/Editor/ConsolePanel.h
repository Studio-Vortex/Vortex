#pragma once

#include "Vortex/Editor/ConsoleMessage.h"

#include <imgui.h>

namespace Vortex {

	class ConsolePanel
	{
	public:
		ConsolePanel();
		~ConsolePanel();

		void OnGuiRender();

		bool& IsOpen() { return s_ShowPanel; }

		bool ClearOnPlay() const { return m_ClearOnPlay; }
		void ClearMessages();

	private:
		void RenderMenu(const ImVec2& size);
		void RenderConsole(const ImVec2& size);
		const char* GetMessageType(const ConsoleMessage& message) const;
		const ImVec4& GetMessageColor(const ConsoleMessage& message) const;
		ImVec4 GetToolbarButtonColor(const bool value) const;

	private:
		static void PushMessage(const ConsoleMessage& message);

	private:
		bool m_ClearOnPlay = true;

		std::mutex m_MessageBufferMutex;
		std::vector<ConsoleMessage> m_MessageBuffer;

		bool m_EnableScrollToLatest = true;
		bool m_ScrollToLatest = false;
		float m_PreviousScrollY = 0.0f;

		int16_t m_MessageFilters = (int16_t)ConsoleMessageFlags::All;

		bool m_DetailedPanelOpen = false;

	private:

		inline static bool s_ShowPanel = false;

		friend class EditorConsoleSink;
	};

}
