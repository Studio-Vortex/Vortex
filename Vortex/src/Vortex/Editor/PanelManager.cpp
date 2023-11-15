#include "vxpch.h"
#include "PanelManager.h"

namespace Vortex {

	bool PanelManager::RemovePanel(EditorPanelType type)
	{
		VX_CORE_ASSERT(m_Panels.contains(type), "Invalid panel type");
		if (!m_Panels.contains(type))
		{
			return false;
		}

		m_Panels.erase(type);
		return true;
	}

    SharedReference<EditorPanel> PanelManager::GetPanel(EditorPanelType type)
    {
		VX_CORE_ASSERT(m_Panels.contains(type), "Invalid panel type");
		if (!m_Panels.contains(type))
		{
			return nullptr;
		}

		return m_Panels[type];
    }

    void PanelManager::OnEditorAttach()
    {
		ForEach([](SharedReference<EditorPanel> panel) {
			panel->OnEditorAttach();
		});
    }

    void PanelManager::OnEditorDetach()
    {
		ForEach([](SharedReference<EditorPanel> panel) {
			panel->OnEditorDetach();
		});
    }

	void PanelManager::SetProjectContext(SharedReference<Project> project)
	{
		ForEach([&](SharedReference<EditorPanel> panel) {
			panel->SetProjectContext(project);
		});
	}

	void PanelManager::SetSceneContext(SharedReference<Scene> scene)
	{
		ForEach([&](SharedReference<EditorPanel> panel) {
			panel->SetSceneContext(scene);
		});
	}

	void PanelManager::ForEach(const EditorPanelFn& func)
	{
		for (auto& [type, panel] : m_Panels)
		{
			func(panel);
		}
	}

	SharedReference<PanelManager> PanelManager::Create()
	{
		return SharedReference<PanelManager>::Create();
	}

}
