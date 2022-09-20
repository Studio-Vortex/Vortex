#pragma once

#include <Sparky.h>

namespace Sparky {

	class StatsPanel
	{
	public:
		StatsPanel() = default;

		void OnGuiRender(Entity hoveredEntity, bool showDefault = false);
		void ShowPanel() { s_ShowPanel = true; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
