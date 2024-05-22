#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class SceneRendererPanel : public EditorPanel
	{
	public:
		~SceneRendererPanel() override = default;

		void OnPanelAttach() override;
		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(SceneRenderer)

	private:
		std::vector<SharedReference<Shader>> m_Loaded2DShaders;
		std::vector<SharedReference<Shader>> m_Loaded3DShaders;
		std::vector<std::string> m_ShaderNames;
		std::vector<SharedReference<Shader>> m_Shaders;
	};

}
