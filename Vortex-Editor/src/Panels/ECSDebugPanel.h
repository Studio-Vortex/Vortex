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
		void RenderSelectedActorView(Actor selectedActor);
		void RenderSceneActorView();

		template <typename... TComponent>
		inline void RenderActorComponentSignature(Actor actor)
		{
			static const char* columns[] = { "Component", "Value" };

			const ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
			const ImVec2 tableSize = { contentRegionAvail.x, 300.0f };
			UI::Table("Component Signature", columns, VX_ARRAYSIZE(columns), tableSize, [&]()
			{
				([&]()
				{
					uint32_t offset = strlen("struct Vortex::");
					const char* componentName = typeid(TComponent).name() + offset;
					const char* hasComponent = actor.HasComponent<TComponent>() ? "true" : "false";
					
					Gui::TableNextColumn();
					Gui::Text(componentName);
					UI::DrawItemActivityOutline();

					if (Gui::IsItemClicked())
					{
						if (actor.HasComponent<TComponent>())
						{
							actor.RemoveComponent<TComponent>();
						}
						else
						{
							actor.AddComponent<TComponent>();
						}
					}

					Gui::TableNextColumn();
					Gui::Text(hasComponent);
				}(), ...);
			});
		}

		template <typename... TComponent>
		inline void RenderActorComponentSignature(ComponentGroup<TComponent...>, Actor selectedActor)
		{
			RenderActorComponentSignature<TComponent...>(selectedActor);
		}

	private:
		std::stack<UUID> m_ClickedActors;

		bool m_ShowActorComponentSignature = false;
	};

}
