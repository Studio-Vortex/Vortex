#pragma once

#include <Vortex.h>

namespace Vortex {

	class NetworkManagerPanel
	{
	public:
		NetworkManagerPanel() = default;
		~NetworkManagerPanel() = default;

		void OnEditorAttach();

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = true;
		SharedReference<Server> m_Server = nullptr;
		ServerProperties m_ServerInfo;
		bool m_ServerRunning = false;
	};

}
