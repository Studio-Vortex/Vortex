#pragma once

#include <Sparky.h>

namespace Sparky {

	class ScriptRegistryPanel
	{
	public:
		ScriptRegistryPanel() = default;

		void OnGuiRender();
		void ShowPanel() { s_ShowPanel = true; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
