#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ECSDebugPanel : public EditorPanel
	{
	public:
		ECSDebugPanel() = default;
		~ECSDebugPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(ECSDebug)

	private:
		void RenderSelectedEntityView(Entity selectedEntity);
		void RenderSceneEntityView();

		template <typename... TComponent>
		inline void RenderEntityComponentSignature(Entity entity)
		{
			static const char* columns[] = { "Component", "Value" };

			UI::Table("Component Signature", columns, VX_ARRAYCOUNT(columns), { Gui::GetContentRegionAvail().x, 400.0f }, [&]()
			{
				([&]()
				{
					uint32_t offset = strlen("struct Vortex::");
					const char* componentName = typeid(TComponent).name() + offset;
					const char* hasComponent = entity.HasComponent<TComponent>() ? "true" : "false";
					
					Gui::TableNextColumn();
					Gui::Text(componentName);
					Gui::TableNextColumn();
					Gui::Text(hasComponent);
				}(), ...);
			});
		}

		template <typename... TComponent>
		inline void RenderEntityComponentSignature(ComponentGroup<TComponent...>, Entity selectedEntity)
		{
			RenderEntityComponentSignature<TComponent...>(selectedEntity);
		}

	private:
		bool m_ShowEntityComponentSignature = false;
	};

}
