#pragma once

#include <Vortex.h>

namespace Vortex {

	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;

		void OnGuiRender(Entity selectedEntity, bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
