#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class NetworkManagerPanel : public EditorPanel
	{
	public:
		~NetworkManagerPanel() override = default;

		void OnEditorAttach() override;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(NetworkManager)

	private:
		SharedReference<Server> m_Server = nullptr;
		ServerProperties m_ServerInfo;
		bool m_ServerRunning = false;
	};

}
