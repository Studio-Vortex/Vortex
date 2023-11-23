#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class AudioMixerPanel : public EditorPanel
	{
	public:
		AudioMixerPanel() = default;
		~AudioMixerPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(AudioMixer)
	};

}
