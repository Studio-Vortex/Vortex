#pragma once

#include <Vortex.h>

namespace Vortex {

	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;
		~MaterialEditorPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderMeshMaterial(Entity selectedEntity);
		void ParameterCallback(SharedReference<Material>& material, uint32_t materialIndex);

	private:
		inline static bool s_ShowPanel = true;
	};

}
