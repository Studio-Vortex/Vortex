#pragma once

#include <Vortex.h>

namespace Vortex {

	class ConsolePanel
	{
	public:
		ConsolePanel() = default;

		void OnGuiRender();
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
