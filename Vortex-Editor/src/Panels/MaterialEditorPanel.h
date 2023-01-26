#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class MaterialEditorPanel : public EditorPanel
	{
	public:
		MaterialEditorPanel() = default;
		~MaterialEditorPanel() override = default;

		void OnGuiRender(Entity selectedEntity);
		void SetProjectContext(SharedRef<Project> project) override {}
		void SetSceneContext(SharedRef<Scene> scene) override {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void MaterialParameterCallback(SharedRef<Material> material, uint32_t materialIndex);

	private:
		inline static bool s_ShowPanel = true;
	};

}
