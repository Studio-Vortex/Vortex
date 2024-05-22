#include "vxpch.h"
#include "SelectionManager.h"

namespace Vortex {

	Actor& SelectionManager::GetSelectedActor()
	{
		return s_SelectedActor;
	}

	Actor& SelectionManager::SetSelectedActor(Actor actor)
	{
		s_SelectedActor = actor;
		return GetSelectedActor();
	}

	void SelectionManager::DeselectActor()
	{
		s_SelectedActor = Actor{};
	}

}
