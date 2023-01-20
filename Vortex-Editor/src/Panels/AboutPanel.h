#pragma once

#include <Vortex.h>

namespace Vortex {

	class AboutPanel
	{
	public:
		AboutPanel() = default;

		void OnGuiRender();
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
