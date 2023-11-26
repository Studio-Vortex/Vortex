#include "AudioMixerPanel.h"

namespace Vortex {

	void AudioMixerPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);



		Gui::End();
	}

}
