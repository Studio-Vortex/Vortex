#pragma once

#include <Vortex.h>

namespace Vortex {

	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;
		~MaterialEditorPanel() = default;

		void OnGuiRender(Entity selectedEntity);
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderMeshMaterial(Entity selectedEntity);
		void ParameterCallback(SharedRef<Material> material, uint32_t materialIndex);
		void RenderPhysicsMaterial(Entity selectedEntity);

	private:
		inline static bool s_ShowPanel = true;
		inline static bool s_ShowPhysicsMaterial = false;
	};

}
