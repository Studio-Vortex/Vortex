#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Editor/EditorPanel.h"

#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/UI/UI.h"

#include <unordered_map>
#include <functional>
#include <string>

namespace Vortex {

	using EditorPanelFn = std::function<void(SharedReference<EditorPanel>)>;

	class VORTEX_API PanelManager : public RefCounted
	{
	public:
		PanelManager();
		~PanelManager();

		template <typename TPanel, typename ...Args>
		SharedReference<TPanel> AddPanel(const std::string& name, Args&& ...args)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "AddPanel only works with types derived from EditorPanel!");

			VX_CORE_ASSERT(!m_Panels.contains(name), "Panel with name already exists");
			SharedReference<TPanel> panel = SharedReference<TPanel>::Create(std::forward(args)...);
			panel->SetName(name);
			m_Panels[name] = panel;

			return panel;
		}

		bool RemovePanel(const std::string& name);

		template <typename TPanel>
		SharedReference<TPanel> GetPanel()
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "GetPanel only works with types derived from EditorPanel!");

			for (auto& [name, panel] : m_Panels)
			{
				if (!panel.Is<TPanel>())
					continue;

				return panel.As<TPanel>();
			}

			VX_CORE_ASSERT(false, "Invalid panel type");
		}

		void OnEditorAttach();
		void OnEditorDetach();

		template <typename TPanel>
		void SetProjectContext(SharedReference<Project> project)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "SetProjectContext only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			panel->SetProjectContext(project);
		}

		void SetProjectContext(SharedReference<Project> project);

		template <typename TPanel>
		void SetSceneContext(SharedReference<Scene> scene)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "SetSceneContext only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			panel->SetSceneContext(scene);
		}

		void SetSceneContext(SharedReference<Scene> scene);

		template <typename TPanel, typename ...Args>
		void OnGuiRender(Args&& ...args)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "OnGuiRender only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			panel->OnGuiRender();
		}

		template <typename TPanel>
		bool AddMainMenuBarItem()
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "AddMainMenuBarItem only works with type derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			std::string panelName = panel->GetName();
			const bool clicked = Gui::MenuItem(panelName.c_str(), nullptr, &panel->IsOpen());

			if (clicked)
			{
				Gui::CloseCurrentPopup();
			}

			return clicked;
		}

		void ForEach(const EditorPanelFn& func);

		static SharedReference<PanelManager> Create();

	private:
		std::unordered_map<std::string, SharedReference<EditorPanel>> m_Panels;
	};

}
