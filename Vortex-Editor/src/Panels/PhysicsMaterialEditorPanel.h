#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class PhysicsMaterialEditorPanel : public EditorPanel
	{
	public:
		~PhysicsMaterialEditorPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(PhysicsMaterialEditor)

	private:
		void RenderPhysicsMaterial(Actor selectedEntity);
	};

}
