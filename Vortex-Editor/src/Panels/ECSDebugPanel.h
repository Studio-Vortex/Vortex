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

			const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
			const ImVec2 tableSize = { contentRegionAvail.x, 300.0f };
			UI::Table("Component Signature", columns, VX_ARRAYCOUNT(columns), tableSize, [&]()
			{
				([&]()
				{
					uint32_t offset = strlen("struct Vortex::");
					const char* componentName = typeid(TComponent).name() + offset;
					const char* hasComponent = entity.HasComponent<TComponent>() ? "true" : "false";
					
					Gui::TableNextColumn();
					Gui::Text(componentName);
					UI::DrawItemActivityOutline();

					if (Gui::IsItemClicked())
					{
						if (entity.HasComponent<TComponent>())
						{
							entity.RemoveComponent<TComponent>();
						}
						else
						{
							entity.AddComponent<TComponent>();
						}
					}

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
		std::stack<UUID> m_ClickedEntities;

		bool m_ShowEntityComponentSignature = false;
	};

}
