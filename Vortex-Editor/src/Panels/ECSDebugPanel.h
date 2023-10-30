#pragma once

#include <Vortex.h>

namespace Vortex {

	class ECSDebugPanel
	{
	public:
		ECSDebugPanel() = default;
		~ECSDebugPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) { }
		void SetSceneContext(SharedReference<Scene>& scene) { m_ContextScene = scene; }
		bool& IsOpen() { return s_ShowPanel; }

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
		SharedReference<Scene> m_ContextScene = nullptr;
		inline static bool s_ShowPanel = false;
		bool m_ShowEntityComponentSignature = false;
	};

}
