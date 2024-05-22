#include "vxpch.h"
#include "PanelManager.h"

namespace Vortex {

	bool PanelManager::HasPanel(EditorPanelType type) const
	{
		if (type == EditorPanelType::None)
		{
			return false;
		}

		return m_Panels.contains(type);
	}

	bool PanelManager::RemovePanel(EditorPanelType type)
	{
		VX_CORE_ASSERT(HasPanel(type), "Invalid panel type");
		if (!HasPanel(type))
		{
			return false;
		}

		m_Panels.erase(type);
		return true;
	}

    SharedReference<EditorPanel> PanelManager::GetPanel(EditorPanelType type) const
    {
		VX_CORE_ASSERT(HasPanel(type), "Invalid panel type");
		if (!HasPanel(type))
		{
			return nullptr;
		}

		return m_Panels.at(type);
    }

    void PanelManager::OnEditorAttach() const
    {
		ForEach([](SharedReference<EditorPanel> panel)
		{
			panel->OnPanelAttach();
		});
    }

    void PanelManager::OnEditorDestroy() const
    {
		ForEach([](SharedReference<EditorPanel> panel)
		{
			panel->OnPanelDetach();
		});
    }

	void PanelManager::SetProjectContext(SharedReference<Project> project) const
	{
		ForEach([&](SharedReference<EditorPanel> panel)
		{
			panel->SetProjectContext(project);
		});
	}

	void PanelManager::SetSceneContext(SharedReference<Scene> scene) const
	{
		ForEach([&](SharedReference<EditorPanel> panel)
		{
			panel->SetSceneContext(scene);
		});
	}

	void PanelManager::ForEach(const EditorPanelFn& fn) const
	{
		for (const auto& [type, panel] : m_Panels)
		{
			std::invoke(fn, panel);
		}
	}

	SharedReference<PanelManager> PanelManager::Create()
	{
		return SharedReference<PanelManager>::Create();
	}

}
