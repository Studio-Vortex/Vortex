#pragma once

#include <Sparky.h>

namespace Sparky {

	class MaterialViewerPanel
	{
	public:
		MaterialViewerPanel() = default;

		void OnGuiRender(Entity selectedEntity, bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = true;
	};

}
