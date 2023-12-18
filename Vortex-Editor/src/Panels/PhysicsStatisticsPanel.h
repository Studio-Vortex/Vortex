#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class PhysicsStatisticsPanel : public EditorPanel
	{
	public:
		~PhysicsStatisticsPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(PhysicsStats)

	private:
		void RenderPhysicsStats() const;
	};

}
