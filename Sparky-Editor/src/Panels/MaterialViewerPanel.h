#pragma once

#include <Sparky.h>

namespace Sparky {

	class MaterialViewerPanel
	{
	public:
		MaterialViewerPanel() = default;

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
