#pragma once

#include <Sparky.h>

namespace Sparky {

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
