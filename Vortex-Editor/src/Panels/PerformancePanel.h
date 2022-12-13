#pragma once

#include <Vortex.h>

namespace Vortex {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;

		void OnGuiRender(size_t entityCount, bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
