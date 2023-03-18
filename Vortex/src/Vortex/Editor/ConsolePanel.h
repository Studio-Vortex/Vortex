#pragma once

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Editor/ConsoleMessage.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

namespace Vortex {

	class Scene;

	class ConsolePanel
	{
	public:
		ConsolePanel();
		~ConsolePanel();

		void OnGuiRender(SharedReference<Scene>& contextScene);

		bool& IsOpen() { return s_ShowPanel; }

		bool ClearOnPlay() const { return m_ClearOnPlay; }
		void ClearMessages();

	private:
		void RenderMenu(const Math::vec2& size);
		void RenderConsole(const Math::vec2& size, SharedReference<Scene>& contextScene);

		const char* GetMessageType(const ConsoleMessage& message) const;
		const Math::vec4& GetMessageColor(const ConsoleMessage& message) const;
		Math::vec4 GetToolbarButtonColor(const bool value) const;

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
