#pragma once

#include <Sparky.h>

namespace Sparky {

	class AboutPanel
	{
	public:
		AboutPanel() = default;

		void OnGuiRender();
		void ShowPanel() { s_ShowPanel = true; }

	private:
		static bool s_ShowPanel;
	};

}
