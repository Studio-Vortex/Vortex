#include "SystemManagerPanel.h"

namespace Vortex {

	void SystemManagerPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		SystemManager::OnGuiRender();

		Gui::End();
	}

}
