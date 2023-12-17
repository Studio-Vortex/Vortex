#include "vxpch.h"
#include "SelectionManager.h"

namespace Vortex {

	Entity& SelectionManager::GetSelectedEntity()
	{
		return s_SelectedEntity;
	}

	Entity& SelectionManager::SetSelectedEntity(Entity entity)
	{
		s_SelectedEntity = entity;
		return GetSelectedEntity();
	}

	void SelectionManager::DeselectEntity()
	{
		s_SelectedEntity = Entity{};
	}

}
