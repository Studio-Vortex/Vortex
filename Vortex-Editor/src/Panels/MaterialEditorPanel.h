#pragma once

#include <Vortex.h>

namespace Vortex {

	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;
		~MaterialEditorPanel() = default;

		void OnGuiRender(Entity selectedEntity);
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderMeshMaterial(Entity selectedEntity);
		void ParameterCallback(SharedReference<Material>& material, uint32_t materialIndex);
		void RenderPhysicsMaterial(Entity selectedEntity);

	private:
		inline static bool s_ShowPanel = true;
		inline static bool s_ShowPhysicsMaterial = false;
	};

}
