#pragma once

#include <Sparky.h>

namespace Sparky {

	class ConsolePanel
	{
	public:
		ConsolePanel() = default;

		void OnGuiRender();
		void ShowPanel() { s_ShowPanel = true; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
