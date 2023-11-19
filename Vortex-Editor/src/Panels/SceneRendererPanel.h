#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class SceneRendererPanel : public EditorPanel
	{
	public:
		~SceneRendererPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(SceneRenderer)

	private:
		inline static std::vector<SharedReference<Shader>> s_Loaded2DShaders;
		inline static std::vector<SharedReference<Shader>> s_Loaded3DShaders;
	};

}
