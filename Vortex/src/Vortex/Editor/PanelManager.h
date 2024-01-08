#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Editor/EditorPanel.h"

#include "Vortex/Editor/UI/UI.h"

#include "Vortex/ReferenceCounting/RefCounted.h"

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

		bool HasPanel(EditorPanelType type) const;

		template <typename TPanel>
		bool HasPanel() const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "HasPanel only works with types derived from EditorPanel!");

			EditorPanelType type = TPanel::GetStaticType();
			return HasPanel(type);
		}

		template <typename TPanel, typename... TArgs>
		SharedReference<TPanel> AddPanel(TArgs&&... args)
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "AddPanel only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = SharedReference<TPanel>::Create(std::forward<TArgs>(args)...);
			EditorPanelType type = TPanel::GetStaticType();
			const std::string panelName = Utils::EditorPanelTypeToString(type);
			panel->SetName(panelName);

			VX_CORE_ASSERT(!HasPanel(type), "Panel with type already exists");
			m_Panels[type] = panel;

			return panel;
		}

		bool RemovePanel(EditorPanelType type);

		template <typename TPanel>
		SharedReference<TPanel> GetPanel() const
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

		SharedReference<EditorPanel> GetPanel(EditorPanelType type) const;

		void OnEditorAttach() const;
		void OnEditorDestroy() const;

		template <typename TPanel>
		void SetProjectContext(SharedReference<Project> project) const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "SetProjectContext only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			if (!panel)
				return;

			panel->SetProjectContext(project);
		}

		void SetProjectContext(SharedReference<Project> project) const;

		template <typename TPanel>
		void SetSceneContext(SharedReference<Scene> scene) const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "SetSceneContext only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			if (!panel)
				return;

			panel->SetSceneContext(scene);
		}

		void SetSceneContext(SharedReference<Scene> scene) const;

		template <typename TPanel>
		void OnGuiRender() const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "OnGuiRender only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			if (!panel)
				return;

			panel->OnGuiRender();
		}

		template <typename TPanel>
		bool TogglePanelOpen() const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "TogglePanelOpen only works with types derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			if (!panel)
				return false;

			return panel->ToggleOpen();
		}

		template <typename TPanel>
		bool MenuBarItem(const std::string& shortcut = "") const
		{
			static_assert(std::is_base_of<EditorPanel, TPanel>::value, "MainMenuBarItem only works with type derived from EditorPanel!");

			SharedReference<TPanel> panel = GetPanel<TPanel>();
			if (!panel)
				return false;

			const std::string& panelName = panel->GetName();

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

		void ForEach(const EditorPanelFn& fn) const;

		static SharedReference<PanelManager> Create();

	private:
		std::unordered_map<EditorPanelType, SharedReference<EditorPanel>> m_Panels;
	};

}
