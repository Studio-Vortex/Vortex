#pragma once

#include <Vortex.h>

namespace Vortex {

	class AssetManagerPanel
	{
	public:
		AssetManagerPanel() = default;

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
		inline static std::vector<SharedRef<Shader>> s_Loaded2DShaders;
		inline static std::vector<SharedRef<Shader>> s_Loaded3DShaders;
	};

}
