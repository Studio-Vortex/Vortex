#include "vxpch.h"
#include "PanelManager.h"

namespace Vortex {

	PanelManager::PanelManager() { }

	PanelManager::~PanelManager() { }

	bool PanelManager::RemovePanel(const std::string& name)
	{
		VX_CORE_ASSERT(m_Panels.contains(name), "Invalid panel name");
		if (!m_Panels.contains(name))
		{
			return false;
		}

		m_Panels.erase(name);
		return true;
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
		for (auto& [name, panel] : m_Panels)
		{
			func(panel);
		}
	}

	SharedReference<PanelManager> PanelManager::Create()
	{
		return SharedReference<PanelManager>::Create();
	}

}
