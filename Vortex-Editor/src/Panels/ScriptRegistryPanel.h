#pragma once

#include <Vortex.h>

namespace Vortex {

	class ScriptRegistryPanel
	{
	public:
		ScriptRegistryPanel() = default;

		void OnGuiRender();
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
