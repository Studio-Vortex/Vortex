#pragma once

#include <Vortex.h>

namespace Vortex {

	class SceneRendererPanel
	{
	public:
		SceneRendererPanel() = default;
		~SceneRendererPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(const SharedReference<Scene>& scene);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		SharedReference<Scene> m_ContextScene = nullptr;
		inline static bool s_ShowPanel = true;
		inline static std::vector<SharedReference<Shader>> s_Loaded2DShaders;
		inline static std::vector<SharedReference<Shader>> s_Loaded3DShaders;
	};

}
