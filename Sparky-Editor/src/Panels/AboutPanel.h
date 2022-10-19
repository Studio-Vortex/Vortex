#pragma once

#include <Sparky.h>

namespace Sparky {

	class AboutPanel
	{
	public:
		AboutPanel() = default;

		void OnGuiRender(bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
