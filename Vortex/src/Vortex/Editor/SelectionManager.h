#pragma once

#include "Vortex/Scene/Entity.h"

namespace Vortex {

	class SelectionManager
	{
	public:
		static Entity& GetSelectedEntity();
		static void SetSelectedEntity(Entity entity);
		static void DeselectEntity();

	private:
		inline static Entity s_SelectedEntity;
	};

}
