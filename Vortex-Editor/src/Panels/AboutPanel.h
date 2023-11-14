#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class AboutPanel : public EditorPanel
	{
	public:
		AboutPanel() = default;
		~AboutPanel() override = default;

		void OnGuiRender() override;
	};

}
