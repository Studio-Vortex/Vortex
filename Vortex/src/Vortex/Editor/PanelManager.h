#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Editor/EditorPanel.h"

#include "Vortex/UI/UI.h"

#include <unordered_map>
#include <functional>
#include <string>

namespace Vortex {

	using VORTEX_API EditorPanelFn = std::function<void(SharedReference<EditorPanel>)>;

	class VORTEX_API PanelManager : public RefCounted
	{
	public:
		PanelManager() = default;
		~PanelManager() = default;

		template <typename TPanel, typename... TArgs>
		SharedReference<TPanel> AddPanel(TArgs&&... args)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "AddPanel only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = SharedReference<TPanel>::Create(std::forward<TArgs>(args)...);
			EditorPanelType type = TPanel::GetStaticType();
			std::string panelName = Utils::EditorPanelTypeToString(type);
			panel->SetName(panelName);

			VX_CORE_ASSERT(!m_Panels.contains(type), "Panel with type already exists");
			m_Panels[type] = panel;

			return panel;
		}

		bool RemovePanel(EditorPanelType type);

		template <typename TPanel>
		SharedReference<TPanel> GetPanel()
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "GetPanel only works with types derived from EditorPanel!");

			for (const auto& [type, panel] : m_Panels)
			{
				if (type != TPanel::GetStaticType())
					continue;
				
				return panel.As<TPanel>();
			}

			VX_CORE_ASSERT(false, "Invalid panel type");
			return nullptr;
		}

		SharedReference<EditorPanel> GetPanel(EditorPanelType type);

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

		template <typename TPanel>
		void OnGuiRender()
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "OnGuiRender only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			panel->OnGuiRender();
		}

		template <typename TPanel>
		bool MenuBarItem(const std::string& shortcut = "")
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "MainMenuBarItem only works with type derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			std::string panelName = panel->GetName();

			const bool clicked = Gui::MenuItem(
				panelName.c_str(),
				shortcut.empty() ? nullptr : shortcut.c_str(),
				&panel->IsOpen
			);

			if (clicked)
			{
				Gui::CloseCurrentPopup();
			}

			return clicked;
		}

		void ForEach(const EditorPanelFn& func);

		static SharedReference<PanelManager> Create();

	private:
		std::unordered_map<EditorPanelType, SharedReference<EditorPanel>> m_Panels;
	};

}
