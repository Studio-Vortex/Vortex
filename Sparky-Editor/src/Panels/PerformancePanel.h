#pragma once

#include <Sparky.h>

namespace Sparky {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;

		void OnGuiRender(size_t entityCount, bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = true;
	};

}
