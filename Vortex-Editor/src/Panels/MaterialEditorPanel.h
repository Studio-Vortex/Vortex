#pragma once

#include <Vortex.h>

namespace Vortex {

	class MaterialEditorPanel
	{
	public:
		MaterialEditorPanel() = default;

		void OnGuiRender(Entity selectedEntity);
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void MaterialParameterCallback(const SharedRef<Material>& material, uint32_t materialIndex);

	private:
		inline static bool s_ShowPanel = true;
	};

}
