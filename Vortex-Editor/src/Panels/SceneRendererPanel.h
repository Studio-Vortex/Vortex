#pragma once

#include <Vortex.h>

namespace Vortex {

	class SceneRendererPanel
	{
	public:
		SceneRendererPanel() = default;

		void SetContext(const SharedRef<Scene>& context);

		void OnGuiRender(bool showDefault = false);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		SharedRef<Scene> m_ContextScene = nullptr;
		inline static bool s_ShowPanel = false;
		inline static std::vector<SharedRef<Shader>> s_Loaded2DShaders;
		inline static std::vector<SharedRef<Shader>> s_Loaded3DShaders;
	};

}
