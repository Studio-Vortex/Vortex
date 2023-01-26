#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class SceneRendererPanel : public EditorPanel
	{
	public:
		SceneRendererPanel() = default;
		~SceneRendererPanel() override = default;

		void OnGuiRender() override;
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override;
		bool& IsOpen() { return s_ShowPanel; }

	private:
		SharedRef<Scene> m_ContextScene = nullptr;
		inline static bool s_ShowPanel = true;
		inline static std::vector<SharedRef<Shader>> s_Loaded2DShaders;
		inline static std::vector<SharedRef<Shader>> s_Loaded3DShaders;
	};

}
