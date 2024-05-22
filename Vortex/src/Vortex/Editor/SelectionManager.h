#pragma once

#include "Vortex/Scene/Actor.h"

namespace Vortex {

	class SelectionManager
	{
	public:
		static Actor& GetSelectedActor();
		static Actor& SetSelectedActor(Actor actor);
		static void DeselectActor();

	private:
		inline static Actor s_SelectedActor;
	};

}
