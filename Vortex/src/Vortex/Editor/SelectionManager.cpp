#include "vxpch.h"
#include "SelectionManager.h"

namespace Vortex {

	Entity& SelectionManager::GetSelectedEntity()
	{
		return s_SelectedEntity;
	}

	void SelectionManager::SetSelectedEntity(Entity entity)
	{
		s_SelectedEntity = entity;
	}

	void SelectionManager::DeselectEntity()
	{
		s_SelectedEntity = Entity{};
	}

}
