#pragma once

#include <Sparky.h>

namespace Sparky {

	class PerformancePanel
	{
	public:
		PerformancePanel() = default;

		void OnGuiRender(Entity hoveredEntity, bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }

	private:
		inline static bool s_ShowPanel = false;
	};

}